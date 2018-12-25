/*
 * RFID_Reader.c
 *
 *  Created on: 28 de jul de 2017
 *      Author: Samsung
 */

/* Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* FreeRTOS libraries */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* ESP-IDF Libraries */
#include "esp_log.h"
#include "driver/gpio.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "RFID_Reader.h"
#include "RFID_Reader_Cfg.h"
#include "../../../main/ProjectCfg.h"

#include "../../drv/SPI/SPI_Master.h"

#define LOG_HEADER		"[RFID_READER SVC] "
#define DUMP_BUFFER_SIZE					(256)
#define READ_REGISTER_BUFFER_SIZE			(2)
#define PICC_SELECT_BUFFER_SIZE				(9)
#define HALTA_BUFFER_SIZE					(4)
#define AUTHENTICATION_BUFFER_SIZE			(12)
#define CONTROL_BUFFER_SIZE					(2)
#define MIFARE_WRITE_CMD_BUFFER_SIZE		(2)
#define MIFARE_TRANSCEIVE_CMD_BUFFER_SIZE	(18)

#ifdef USING_RFID

/************************************************************************************************/
/************************************** Local Variables *****************************************/
/************************************************************************************************/

static RFID_Reader_t rfidInstance =
{
	.initialized = false,
};
static char dumpBuffer[DUMP_BUFFER_SIZE];										/*!< Buffer used to build dump messages */
static uint8_t readRegisterBuffer[READ_REGISTER_BUFFER_SIZE];					/*!< Buffer used to read PCD register */
static uint8_t piccSelectBuffer[PICC_SELECT_BUFFER_SIZE];						/*!< The SELECT/ANTICOLLISION commands uses a 7 byte standard frame + 2 bytes CRC_A */
static uint8_t haltAbuffer[HALTA_BUFFER_SIZE];									/*!< Buffer used to haltA operation */
static uint8_t authenticationBuffer[AUTHENTICATION_BUFFER_SIZE];				/*!< Buffer used to authentication */
static uint8_t controlBuffer[CONTROL_BUFFER_SIZE]; 								/*!< Buffer used to PICC communication */
static uint8_t mifareWriteCmddBuffer[MIFARE_WRITE_CMD_BUFFER_SIZE];				/*!< Buffer used to tell the PICC we want to write to an block blockAddr */
static uint8_t mifareTransceivecmdBuffer[MIFARE_TRANSCEIVE_CMD_BUFFER_SIZE];	/*!< Buffer used to execute mifare transceive command, We need room for 16 bytes data and 2 bytes CRC_A.*/

/************************************************************************************************/
/******************************* Internal Functions Definition **********************************/
/************************************************************************************************/



/************************************************************************************************/
/******************************* Public API  Implementation *************************************/
/************************************************************************************************/

CubeeErrorCode_t RFID_Reader_init(RFID_Reader_t* this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER initializing RFID driver");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(this->initialized != false)
	{
		/* Already initialized */
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing RFID driver...");

	this->rfidReaderCfg = RFID_Reader_Cfg_getInstance();
	if(RFID_Reader_Cfg_init(this->rfidReaderCfg) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing RFID configuration");
		return CUBEE_ERROR_UNDEFINED;
	}

	/****************** Initialization CODDE *********************/
	this->spi = SPI_getInstance();
	if(SPI_init(this->spi) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing RFID, SPI initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	if(PCD_Init(this) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing RFID, PCD initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/************** END OF Initialization CODDE *****************/

	this->initialized = true;
	ESP_LOGI(LOG_HEADER,"RFID successfully initialized");

	return CUBEE_ERROR_OK;
}

RFID_Reader_Cfg_t * RFID_Reader_getConfig(RFID_Reader_t* this)
{
	if((this == NULL) ||  (this->initialized == false))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER reading RFID configuration");
		return NULL;
	}

	return this->rfidReaderCfg;
}

RFID_Reader_t * RFID_Reader_getInstance()
{
	return &rfidInstance;
}

bool RFID_Reader_isInitialized(RFID_Reader_t* this)
{
	return this->initialized;
}

uint8_t PCD_ReadSingleByteRegister(RFID_Reader_t* this, RFID_PCD_Register_t reg)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER reading RFID PCD single byte register");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Clean buffer */
	memset(readRegisterBuffer,0,READ_REGISTER_BUFFER_SIZE);

	readRegisterBuffer[0] = reg | 0x80;
	readRegisterBuffer[1] = 0;

	this->spi->dataTransferBuffer = readRegisterBuffer;
	this->spi->dataReceiveBuffer = readRegisterBuffer;
	this->spi->dataTransferBufferSize = 2;

	if(SPI_transferData(this->spi,SPI_SLAVE_ID_RFID) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error reading RFID PCD single byte register");
		return UINT8_MAX;
	}

	return readRegisterBuffer[1];
}

CubeeErrorCode_t PCD_ReadMultipleByteRegister(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t count, uint8_t *values, uint8_t rxAlign)
{
	uint8_t address;
	uint8_t index;
	uint8_t mask;
	uint8_t value;

	if((this == NULL) || (values == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER reading PCD multiple byte register, this == null  (%d), values == NULL (%d)", (this == NULL) , (values == NULL));
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(count == 0)
	{
		ESP_LOGI(LOG_HEADER, "Error reading multiple byte register, input size == 0");
		return CUBEE_ERROR_OK;
	}

	// Tell MFRC522 which address we want to read
	address = 0x80 | reg;				// MSB == 1 is for reading. LSB is not used in address. Datasheet section 8.1.2.3.
	this->spi->dataTransferBuffer = &address;
	this->spi->dataReceiveBuffer = NULL;
	this->spi->dataTransferBufferSize = 1;

	if(SPI_transferData(this->spi,SPI_SLAVE_ID_RFID) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error reading RFID PCD multiple byte register");
		return CUBEE_ERROR_UNDEFINED;
	}

	index = 0;							// Index in values array.
	count--;							// One read is performed outside of the loop
	// Only update bit positions rxAlign..7 in values[0]
	if (rxAlign)
	{
		// Create bit mask for bit positions rxAlign..7
		mask = (0xFF << rxAlign) & 0xFF;
		// Read value and tell that we want to read the same address again.
		address = 0x80 | reg;
		this->spi->dataReceiveBuffer = &value;
		if(SPI_transferData(this->spi,SPI_SLAVE_ID_RFID) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER, "Error reading RFID PCD multiple byte register");
			return CUBEE_ERROR_UNDEFINED;
		}
		// Apply mask to both current value of values[0] and the new data in value.
		values[0] = (values[0] & ~mask) | (value & mask);
		index++;
	}
	while (index < count)
	{
		// Read value and tell that we want to read the same address again.
		address = 0x80 | reg;
		this->spi->dataReceiveBuffer = &values[index];
		if(SPI_transferData(this->spi,SPI_SLAVE_ID_RFID) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER, "Error reading RFID PCD multiple byte register");
			return CUBEE_ERROR_UNDEFINED;
		}
		index++;
	}

	// Read the final byte. Send 0 to stop reading.
	address = 0;
	this->spi->dataReceiveBuffer = &values[index];
	if(SPI_transferData(this->spi,SPI_SLAVE_ID_RFID) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error reading RFID PCD multiple byte register");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t PCD_WriteSingleByteRegister(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t value)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER writing PCD single byte register");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Clean write register buffer */
	memset(this->writeRegisterBuffer,0,RFID_WRITE_REGISTER_BUFF_MAX_SIZE);

	this->writeRegisterBuffer[0] = reg;
	this->writeRegisterBuffer[1] = value;

	this->spi->dataTransferBuffer = this->writeRegisterBuffer;
	this->spi->dataReceiveBuffer = this->writeRegisterBuffer;
	this->spi->dataTransferBufferSize = 2;

	return SPI_transferData(this->spi,SPI_SLAVE_ID_RFID);
}

CubeeErrorCode_t PCD_WriteMultipleByteRegister(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t count, uint8_t *values)
{
	if((this == NULL) || (count == 0) || (values == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER writing PCD multiple byte register");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Clean write register buffer */
	memset(this->writeRegisterBuffer,0,RFID_WRITE_REGISTER_BUFF_MAX_SIZE);

	this->writeRegisterBuffer[0] = reg;
	memcpy(this->writeRegisterBuffer + 1, values, count);

	this->spi->dataTransferBuffer = this->writeRegisterBuffer;
	this->spi->dataReceiveBuffer = this->writeRegisterBuffer;
	this->spi->dataTransferBufferSize = count+1;

	return SPI_transferData(this->spi,SPI_SLAVE_ID_RFID);
}


CubeeErrorCode_t PCD_SetRegisterBitMask(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t mask)
{
	uint8_t regValue;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER Setting register bit");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	regValue = PCD_ReadSingleByteRegister(this, reg);
	// set bit mask
	return PCD_WriteSingleByteRegister(this, reg, regValue | mask);
}

CubeeErrorCode_t PCD_ClearRegisterBitMask(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t mask)
{
	uint8_t regValue;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER Clearing register bit");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	regValue = PCD_ReadSingleByteRegister(this, reg);
	// clear bit mask
	return PCD_WriteSingleByteRegister(this, reg, regValue & (~mask));
}

RFID_StatusCode_t PCD_CalculateCRC(RFID_Reader_t* this, uint8_t *data, uint8_t length, uint8_t *result)
{
	if((this == NULL) || (result == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER calculating CRC");
		return STATUS_INTERNAL_ERROR;
	}

	/* TODO: Verify errors */
	PCD_WriteSingleByteRegister(this, CommandReg, PCD_Idle);		// Stop any active command.
	PCD_WriteSingleByteRegister(this, DivIrqReg, 0x04);				// Clear the CRCIRq interrupt request bit
	PCD_WriteSingleByteRegister(this, FIFOLevelReg, 0x80);			// FlushBuffer = 1, FIFO initialization
	PCD_WriteMultipleByteRegister(this, FIFODataReg, length, data);			// Write data to the FIFO
	PCD_WriteSingleByteRegister(this, CommandReg, PCD_CalcCRC);		// Start the calculation

	// Wait for the CRC calculation to complete
	vTaskDelay(100 / portTICK_PERIOD_MS);
	uint8_t n = PCD_ReadSingleByteRegister(this, DivIrqReg);
	if (n & 0x04)
	{
		// CRCIRq bit set - calculation done
		PCD_WriteSingleByteRegister(this, CommandReg, PCD_Idle);	// Stop calculating CRC for new content in the FIFO.
		// Transfer the result from the registers to the result buffer
		result[0] = PCD_ReadSingleByteRegister(this, CRCResultRegL);
		result[1] = PCD_ReadSingleByteRegister(this, CRCResultRegH);
		return STATUS_OK;
	}

	// 89ms passed and nothing happend. Communication with the MFRC522 might be down.
	return STATUS_TIMEOUT;

}

CubeeErrorCode_t PCD_Init(RFID_Reader_t* this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER initializing PCD");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Soft reset */
	if(PCD_Reset(this) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, soft reset failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Reset baud rates */
	if(PCD_WriteSingleByteRegister(this, TxModeReg, 0x00) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, Reset Rx baud rate failed");
		return CUBEE_ERROR_UNDEFINED;
	}
	if(PCD_WriteSingleByteRegister(this, RxModeReg, 0x00) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, Reset Tx baud rate failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Reset ModWidthReg */
	if(PCD_WriteSingleByteRegister(this, ModWidthReg, 0x26) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, ModWidthReg reset failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/**
	 * When communicating with a PICC we need a timeout if something goes wrong.
	 * TPrescaler_Hi are the four low bits in TModeReg. TPrescaler_Lo is TPrescalerReg.
	 */
	if(PCD_WriteSingleByteRegister(this, TModeReg, 0x80) != CUBEE_ERROR_OK) // TAuto=1; timer starts automatically at the end of the transmission in all communication modes at all speeds
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, timer configuration failed");
		return CUBEE_ERROR_UNDEFINED;
	}
	if(PCD_WriteSingleByteRegister(this, TPrescalerReg, 0xA9) != CUBEE_ERROR_OK) // TPreScaler = TModeReg[3..0]:TPrescalerReg, ie 0x0A9 = 169 => f_timer=40kHz, ie a timer period of 25us.
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, Timer frequency configuration failed");
		return CUBEE_ERROR_UNDEFINED;
	}
	if(PCD_WriteSingleByteRegister(this, TReloadRegH, 0x03) != CUBEE_ERROR_OK) // Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, timer reload high configuration failed");
		return CUBEE_ERROR_UNDEFINED;
	}
	if(PCD_WriteSingleByteRegister(this, TReloadRegL, 0xE8) != CUBEE_ERROR_OK) // Reload timer with 0x3E8 = 1000, ie 25ms before timeout.
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, timer reload low configuration failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Default 0x00. Force a 100 % ASK modulation independent of the ModGsPReg register setting */
	if(PCD_WriteSingleByteRegister(this, TxASKReg, 0x40) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, ASK modulation setting failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Default 0x3F. Set the preset value for the CRC coprocessor for the CalcCRC command to 0x6363 (ISO 14443-3 part 6.2.4) */
	if(PCD_WriteSingleByteRegister(this, ModeReg, 0x3D) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, CRC configuration failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Enable the antenna driver pins TX1 and TX2 (they were disabled by the reset) */
	if(PCD_AntennaOn(this) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing PCD, Antenna start failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t PCD_Reset(RFID_Reader_t* this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER reseting PCD");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Issue the SoftReset command. */
	if(PCD_WriteSingleByteRegister(this,CommandReg, PCD_SoftReset) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error reseting PCD, command send failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/**
	 *  The datasheet does not mention how long the SoftRest command takes to complete.
	 *  But the MFRC522 might have been in soft power-down mode (triggered by bit 4 of CommandReg)
	 *  Section 8.8.2 in the datasheet says the oscillator start-up time is the start up time of the crystal + 37,74us. Let us be generous: 50ms.
	 */
	vTaskDelay(50 / portTICK_PERIOD_MS);

	/* Wait for the PowerDown bit in CommandReg to be cleared */
	while (PCD_ReadSingleByteRegister(this,CommandReg) & (1<<4)) {
		// PCD still restarting - unlikely after waiting 50ms, but better safe than sorry.
	}

	ESP_LOGI(LOG_HEADER,"RFID PCD soft reset executed");

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t PCD_AntennaOn(RFID_Reader_t* this)
{
	uint8_t value;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER turning PCD antenna on");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	value = PCD_ReadSingleByteRegister(this,TxControlReg);
	if ((value & 0x03) != 0x03)
	{
		return PCD_WriteSingleByteRegister(this, TxControlReg, value | 0x03);
	}

	return CUBEE_ERROR_OK;
}

RFID_StatusCode_t PCD_TransceiveData(RFID_Reader_t* this, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC)
{
	uint8_t waitIRq = 0x30;		// RxIRq and IdleIRq

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER sending PCD transceive command ");
		return STATUS_INTERNAL_ERROR;
	}

	return PCD_CommunicateWithPICC(this, PCD_Transceive, waitIRq, sendData, sendLen, backData, backLen, validBits, rxAlign, checkCRC);
}

RFID_StatusCode_t PCD_CommunicateWithPICC(RFID_Reader_t* this, uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC)
{
	uint8_t txLastBits;
	uint8_t bitFraming;
	uint8_t errorRegValue;
	uint8_t _validBits;
	RFID_StatusCode_t status;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER communicating with PICC ");
		return STATUS_INVALID;
	}

	/* Clean buffer */
	memset(controlBuffer,0,CONTROL_BUFFER_SIZE);

	// Prepare values for BitFramingReg
	txLastBits = validBits ? *validBits : 0;
	bitFraming = (rxAlign << 4) + txLastBits;		// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]


	// Stop any active command.
	if(PCD_WriteSingleByteRegister(this, CommandReg, PCD_Idle) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error communication with PICC, Stop any active command failed");
		return STATUS_INTERNAL_ERROR;
	}
	// Clear all seven interrupt request bits
	if(PCD_WriteSingleByteRegister(this, ComIrqReg, 0x7F) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error communication with PICC, Clear all seven interrupt request bits failed");
		return STATUS_INTERNAL_ERROR;
	}
	// FlushBuffer = 1, FIFO initialization
	if(PCD_WriteSingleByteRegister(this, FIFOLevelReg, 0x80) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error communication with PICC, FIFO initialization failed");
		return STATUS_INTERNAL_ERROR;
	}
	// Write sendData to the FIFO
	if(PCD_WriteMultipleByteRegister(this, FIFODataReg, sendLen, sendData) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error communication with PICC, Write sendData to the FIFO failed");
		return STATUS_INTERNAL_ERROR;
	}
	// Bit adjustments
	if(PCD_WriteSingleByteRegister(this, BitFramingReg, bitFraming) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error communication with PICC, Bit adjustments failed");
		return STATUS_INTERNAL_ERROR;
	}
	// Execute the command
	if(PCD_WriteSingleByteRegister(this, CommandReg,  command) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error communication with PICC, Execute the command failed");
		return STATUS_INTERNAL_ERROR;
	}
	if (command == PCD_Transceive)
	{
		// StartSend=1, transmission of data starts
		if(PCD_SetRegisterBitMask(this, BitFramingReg, 0x80) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER, "Error communication with PICC, Execute the command to start transmission of data failed");
			return STATUS_INTERNAL_ERROR;
		}
	}

	// Wait for the command to complete.
	// In PCD_Init() we set the TAuto flag in TModeReg. This means the timer automatically starts when the PCD stops transmitting.
	// Each iteration of the do-while-loop takes 17.86us.
	vTaskDelay(40 / portTICK_PERIOD_MS);
	uint8_t n = PCD_ReadSingleByteRegister(this, ComIrqReg);	// ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq HiAlertIRq LoAlertIRq ErrIRq TimerIRq
	if ((n & 0x01) || (!(n & waitIRq))) {
		// Nothing received in 25ms, or no interrupts has been set.
		return STATUS_TIMEOUT;
	}

	// Stop now if any errors except collisions were detected.
	errorRegValue = PCD_ReadSingleByteRegister(this,ErrorReg); // ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl CollErr CRCErr ParityErr ProtocolErr
	if (errorRegValue & 0x13)
	{	 // BufferOvfl ParityErr ProtocolErr
		return STATUS_ERROR;
	}

	_validBits = 0;

	// If the caller wants data back, get it from the MFRC522.
	if (backData && backLen)
	{
		uint8_t n = PCD_ReadSingleByteRegister(this, FIFOLevelReg);	// Number of bytes in the FIFO
		if (n > *backLen) {
			return STATUS_NO_ROOM;
		}
		else if(n==0)
		{
			ESP_LOGE(LOG_HEADER, "PCD - PICC communication, PCD FIFO is empty");
		}
		else
		{
			PCD_ReadMultipleByteRegister(this, FIFODataReg, n, backData, rxAlign);	// Get received data from FIFO
		}

		*backLen = n;
		_validBits = PCD_ReadSingleByteRegister(this,ControlReg) & 0x07;		// RxLastBits[2:0] indicates the number of valid bits in the last received byte. If this value is 000b, the whole byte is valid.
		if (validBits) {
			*validBits = _validBits;
		}
	}

	// Tell about collisions
	if (errorRegValue & 0x08) {		// CollErr
		return STATUS_COLLISION;
	}

	// Perform CRC_A validation if requested.
	if (backData && backLen && checkCRC) {
		// In this case a MIFARE Classic NAK is not OK.
		if (*backLen == 1 && _validBits == 4) {
			return STATUS_MIFARE_NACK;
		}
		// We need at least the CRC_A value and all 8 bits of the last byte must be received.
		if (*backLen < 2 || _validBits != 0) {
			return STATUS_CRC_WRONG;
		}
		// Verify CRC_A - do our own calculation and store the control in controlBuffer.
		status = PCD_CalculateCRC(this, &backData[0], *backLen - 2, &controlBuffer[0]);
		if (status != STATUS_OK) {
			return status;
		}
		if ((backData[*backLen - 2] != controlBuffer[0]) || (backData[*backLen - 1] != controlBuffer[1])) {
			return STATUS_CRC_WRONG;
		}
	}

	return STATUS_OK;
}

RFID_StatusCode_t PICC_RequestA(RFID_Reader_t* this, uint8_t *bufferATQA, uint8_t *bufferSize)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER Transmitting a REQuest command Type A");
		return STATUS_INTERNAL_ERROR;
	}

	return PICC_REQA_or_WUPA(this, PICC_CMD_REQA, bufferATQA, bufferSize);
}

RFID_StatusCode_t PICC_REQA_or_WUPA(RFID_Reader_t* this, uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize)
{
	uint8_t validBits;
	RFID_StatusCode_t status;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER Transmitting REQA or WUPA command");
		return STATUS_INTERNAL_ERROR;
	}
	else if (bufferATQA == NULL || *bufferSize < 2)
	{
		// The ATQA response is 2 bytes long.
		return STATUS_NO_ROOM;
	}

	// ValuesAfterColl=1 => Bits received after collision are cleared.
	if(PCD_ClearRegisterBitMask(this, CollReg, 0x80) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error transmitting REQA or WUPA command, collision bit mask clearing failed");
		return STATUS_INTERNAL_ERROR;
	}

	// For REQA and WUPA we need the short frame format - transmit only 7 bits of the last (and only) byte. TxLastBits = BitFramingReg[2..0]
	validBits = 7;
	status = PCD_TransceiveData(this, &command, 1, bufferATQA, bufferSize, &validBits, 0, false);

	if (status != STATUS_OK) {
		return status;
	}
	// ATQA must be exactly 16 bits.
	if (*bufferSize != 2 || validBits != 0)
	{
		ESP_LOGE(LOG_HEADER, "Error transmitting REQA or WUPA command, invalid result, bufferSize = %d, validBits = %d", *bufferSize , validBits);
		return STATUS_ERROR;
	}

	return STATUS_OK;
}

RFID_StatusCode_t PICC_Select(RFID_Reader_t* this, RFID_PICC_Uid_t *uid, uint8_t validBits)
{
	bool uidComplete;
	bool selectDone;
	bool useCascadeTag;
	uint8_t cascadeLevel = 1;
	RFID_StatusCode_t result;
	uint8_t count;
	uint8_t index;
	uint8_t uidIndex;					// The first index in uid->uidByte[] that is used in the current Cascade Level.
	int8_t currentLevelKnownBits;		// The number of known UID bits in the current Cascade Level.
	uint8_t bufferUsed;					// The number of bytes used in the buffer, ie the number of bytes to transfer to the FIFO.
	uint8_t rxAlign;					// Used in BitFramingReg. Defines the bit position for the first bit received.
	uint8_t txLastBits;					// Used in BitFramingReg. The number of valid bits in the last transmitted byte.
	uint8_t *responseBuffer;
	uint8_t responseLength;

	/* Clean buffer */
	memset(piccSelectBuffer,0,PICC_SELECT_BUFFER_SIZE);

	// Description of buffer structure:
	//		Byte 0: SEL 				Indicates the Cascade Level: PICC_CMD_SEL_CL1, PICC_CMD_SEL_CL2 or PICC_CMD_SEL_CL3
	//		Byte 1: NVB					Number of Valid Bits (in complete command, not just the UID): High nibble: complete bytes, Low nibble: Extra bits.
	//		Byte 2: UID-data or CT		See explanation below. CT means Cascade Tag.
	//		Byte 3: UID-data
	//		Byte 4: UID-data
	//		Byte 5: UID-data
	//		Byte 6: BCC					Block Check Character - XOR of bytes 2-5
	//		Byte 7: CRC_A
	//		Byte 8: CRC_A
	// The BCC and CRC_A are only transmitted if we know all the UID bits of the current Cascade Level.
	//
	// Description of bytes 2-5: (Section 6.5.4 of the ISO/IEC 14443-3 draft: UID contents and cascade levels)
	//		UID size	Cascade level	Byte2	Byte3	Byte4	Byte5
	//		========	=============	=====	=====	=====	=====
	//		 4 bytes		1			uid0	uid1	uid2	uid3
	//		 7 bytes		1			CT		uid0	uid1	uid2
	//						2			uid3	uid4	uid5	uid6
	//		10 bytes		1			CT		uid0	uid1	uid2
	//						2			CT		uid3	uid4	uid5
	//						3			uid6	uid7	uid8	uid9

	// Sanity checks
	if (validBits > 80) {
		return STATUS_INVALID;
	}

	// Prepare MFRC522
	PCD_ClearRegisterBitMask(this, CollReg, 0x80);		// ValuesAfterColl=1 => Bits received after collision are cleared.

	// Repeat Cascade Level loop until we have a complete UID.
	uidComplete = false;
	while (!uidComplete) {
		// Set the Cascade Level in the SEL byte, find out if we need to use the Cascade Tag in byte 2.
		switch (cascadeLevel) {
			case 1:
				piccSelectBuffer[0] = PICC_CMD_SEL_CL1;
				uidIndex = 0;
				useCascadeTag = validBits && uid->size > 4;	// When we know that the UID has more than 4 bytes
				break;

			case 2:
				piccSelectBuffer[0] = PICC_CMD_SEL_CL2;
				uidIndex = 3;
				useCascadeTag = validBits && uid->size > 7;	// When we know that the UID has more than 7 bytes
				break;

			case 3:
				piccSelectBuffer[0] = PICC_CMD_SEL_CL3;
				uidIndex = 6;
				useCascadeTag = false;						// Never used in CL3.
				break;

			default:
				return STATUS_INTERNAL_ERROR;
				break;
		}

		// How many UID bits are known in this Cascade Level?
		currentLevelKnownBits = validBits - (8 * uidIndex);
		if (currentLevelKnownBits < 0) {
			currentLevelKnownBits = 0;
		}
		// Copy the known bits from uid->uidByte[] to buffer[]
		index = 2; // destination index in buffer[]
		if (useCascadeTag) {
			piccSelectBuffer[index++] = PICC_CMD_CT;
		}
		uint8_t bytesToCopy = currentLevelKnownBits / 8 + (currentLevelKnownBits % 8 ? 1 : 0); // The number of bytes needed to represent the known bits for this level.
		if (bytesToCopy) {
			uint8_t maxBytes = useCascadeTag ? 3 : 4; // Max 4 bytes in each Cascade Level. Only 3 left if we use the Cascade Tag
			if (bytesToCopy > maxBytes) {
				bytesToCopy = maxBytes;
			}
			for (count = 0; count < bytesToCopy; count++) {
				piccSelectBuffer[index++] = uid->uidByte[uidIndex + count];
			}
		}
		// Now that the data has been copied we need to include the 8 bits in CT in currentLevelKnownBits
		if (useCascadeTag) {
			currentLevelKnownBits += 8;
		}

		// Repeat anti collision loop until we can transmit all UID bits + BCC and receive a SAK - max 32 iterations.
		selectDone = false;
		while (!selectDone) {
			// Find out how many bits and bytes to send and receive.
			if (currentLevelKnownBits >= 32) { // All UID bits in this Cascade Level are known. This is a SELECT.
				//Serial.print(F("SELECT: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
				piccSelectBuffer[1] = 0x70; // NVB - Number of Valid Bits: Seven whole bytes
				// Calculate BCC - Block Check Character
				piccSelectBuffer[6] = piccSelectBuffer[2] ^ piccSelectBuffer[3] ^ piccSelectBuffer[4] ^ piccSelectBuffer[5];
				// Calculate CRC_A
				result = PCD_CalculateCRC(this, piccSelectBuffer, 7, &piccSelectBuffer[7]);
				if (result != STATUS_OK) {
					return result;
				}
				txLastBits		= 0; // 0 => All 8 bits are valid.
				bufferUsed		= 9;
				// Store response in the last 3 bytes of buffer (BCC and CRC_A - not needed after tx)
				responseBuffer	= &piccSelectBuffer[6];
				responseLength	= 3;
			}
			else { // This is an ANTICOLLISION.
				//Serial.print(F("ANTICOLLISION: currentLevelKnownBits=")); Serial.println(currentLevelKnownBits, DEC);
				txLastBits		= currentLevelKnownBits % 8;
				count			= currentLevelKnownBits / 8;	// Number of whole bytes in the UID part.
				index			= 2 + count;					// Number of whole bytes: SEL + NVB + UIDs
				piccSelectBuffer[1]		= (index << 4) + txLastBits;	// NVB - Number of Valid Bits
				bufferUsed		= index + (txLastBits ? 1 : 0);
				// Store response in the unused part of buffer
				responseBuffer	= &piccSelectBuffer[index];
				responseLength	= PICC_SELECT_BUFFER_SIZE - index;
			}

			// Set bit adjustments
			rxAlign = txLastBits;											// Having a separate variable is overkill. But it makes the next line easier to read.
			PCD_WriteSingleByteRegister(this, BitFramingReg, (rxAlign << 4) + txLastBits);	// RxAlign = BitFramingReg[6..4]. TxLastBits = BitFramingReg[2..0]

			// Transmit the buffer and receive the response.
			result = PCD_TransceiveData(this, piccSelectBuffer, bufferUsed, responseBuffer, &responseLength, &txLastBits, rxAlign, false);
			if (result == STATUS_COLLISION) { // More than one PICC in the field => collision.
				uint8_t valueOfCollReg = PCD_ReadSingleByteRegister(this, CollReg); // CollReg[7..0] bits are: ValuesAfterColl reserved CollPosNotValid CollPos[4:0]
				if (valueOfCollReg & 0x20) { // CollPosNotValid
					return STATUS_COLLISION; // Without a valid collision position we cannot continue
				}
				uint8_t collisionPos = valueOfCollReg & 0x1F; // Values 0-31, 0 means bit 32.
				if (collisionPos == 0) {
					collisionPos = 32;
				}
				if (collisionPos <= currentLevelKnownBits) { // No progress - should not happen
					return STATUS_INTERNAL_ERROR;
				}
				// Choose the PICC with the bit set.
				currentLevelKnownBits = collisionPos;
				count			= (currentLevelKnownBits - 1) % 8; // The bit to modify
				index			= 1 + (currentLevelKnownBits / 8) + (count ? 1 : 0); // First byte is index 0.
				piccSelectBuffer[index]	|= (1 << count);
			}
			else if (result != STATUS_OK) {
				return result;
			}
			else { // STATUS_OK
				if (currentLevelKnownBits >= 32) { // This was a SELECT.
					selectDone = true; // No more anticollision
					// We continue below outside the while.
				}
				else { // This was an ANTICOLLISION.
					// We now have all 32 bits of the UID in this Cascade Level
					currentLevelKnownBits = 32;
					// Run loop again to do the SELECT.
				}
			}
		} // End of while (!selectDone)

		// We do not check the CBB - it was constructed by us above.

		// Copy the found UID bytes from buffer[] to uid->uidByte[]
		index			= (piccSelectBuffer[2] == PICC_CMD_CT) ? 3 : 2; // source index in buffer[]
		bytesToCopy		= (piccSelectBuffer[2] == PICC_CMD_CT) ? 3 : 4;
		for (count = 0; count < bytesToCopy; count++) {
			uid->uidByte[uidIndex + count] = piccSelectBuffer[index++];
		}

		// Check response SAK (Select Acknowledge)
		if (responseLength != 3 || txLastBits != 0) { // SAK must be exactly 24 bits (1 byte + CRC_A).
			return STATUS_ERROR;
		}
		// Verify CRC_A - do our own calculation and store the control in buffer[2..3] - those bytes are not needed anymore.
		result = PCD_CalculateCRC(this, responseBuffer, 1, &piccSelectBuffer[2]);
		if (result != STATUS_OK) {
			return result;
		}
		if ((piccSelectBuffer[2] != responseBuffer[1]) || (piccSelectBuffer[3] != responseBuffer[2])) {
			return STATUS_CRC_WRONG;
		}
		if (responseBuffer[0] & 0x04) { // Cascade bit set - UID not complete yes
			cascadeLevel++;
		}
		else {
			uidComplete = true;
			uid->sak = responseBuffer[0];
		}
	} // End of while (!uidComplete)

	// Set correct uid->size
	uid->size = 3 * cascadeLevel + 1;

	return STATUS_OK;
}

RFID_StatusCode_t PICC_HaltA(RFID_Reader_t* this)
{
	RFID_StatusCode_t result;

	/* Clean buffer */
	memset(haltAbuffer,0,HALTA_BUFFER_SIZE);

	// Build command buffer
	haltAbuffer[0] = PICC_CMD_HLTA;
	haltAbuffer[1] = 0;
	// Calculate CRC_A
	result = PCD_CalculateCRC(this, haltAbuffer, 2, &haltAbuffer[2]);
	if (result != STATUS_OK)
	{
		return result;
	}

	// Send the command.
	// The standard says:
	//		If the PICC responds with any modulation during a period of 1 ms after the end of the frame containing the
	//		HLTA command, this response shall be interpreted as 'not acknowledge'.
	// We interpret that this way: Only STATUS_TIMEOUT is a success.
	result = PCD_TransceiveData(this, haltAbuffer, HALTA_BUFFER_SIZE, NULL, 0, NULL, 0, false);
	if (result == STATUS_TIMEOUT)
	{
		return STATUS_OK;
	}
	if (result == STATUS_OK)
	{ // That is ironically NOT ok in this case ;-)
		return STATUS_ERROR;
	}
	return result;
}

RFID_StatusCode_t PCD_Authenticate(RFID_Reader_t* this, uint8_t command, uint8_t blockAddr, RFID_MIFARE_Key_t *key, RFID_PICC_Uid_t *uid)
{
	RFID_StatusCode_t status;
	uint8_t waitIRq = 0x10;		// IdleIRq

	if((this == NULL) || (key == NULL) || (uid == NULL))
	{
		ESP_LOGE(LOG_HEADER, "ERROR_INVALID_PARAMETER on PCD authentication");
		return STATUS_INVALID;
	}

	/* Clean buffer */
	memset(authenticationBuffer,0,AUTHENTICATION_BUFFER_SIZE);

	// Build command buffer
	authenticationBuffer[0] = command;
	authenticationBuffer[1] = blockAddr;
	for (uint8_t i = 0; i < MF_KEY_SIZE; i++) {	// 6 key bytes
		authenticationBuffer[2+i] = key->keyByte[i];
	}
	// Use the last uid bytes as specified in http://cache.nxp.com/documents/application_note/AN10927.pdf
	// section 3.2.5 "MIFARE Classic Authentication".
	// The only missed case is the MF1Sxxxx shortcut activation,
	// but it requires cascade tag (CT) byte, that is not part of uid.
	for (uint8_t i = 0; i < 4; i++)
	{	// The last 4 bytes of the UID
		authenticationBuffer[8+i] = uid->uidByte[i+uid->size-4];
	}

	status = PCD_CommunicateWithPICC(this, PCD_MFAuthent, waitIRq, &authenticationBuffer[0], AUTHENTICATION_BUFFER_SIZE, NULL, NULL, NULL, 0, false);

	return status;
}

CubeeErrorCode_t PCD_StopCrypto1(RFID_Reader_t* this)
{
	// Clear MFCrypto1On bit
	return PCD_ClearRegisterBitMask(this, Status2Reg, 0x08); // Status2Reg[7..0] bits are: TempSensClear I2CForceHS reserved reserved MFCrypto1On ModemState[2:0]
}

RFID_StatusCode_t MIFARE_Read(RFID_Reader_t* this, uint8_t blockAddr, uint8_t *buffer, uint8_t *bufferSize)
{
	RFID_StatusCode_t result;

	// Sanity check
	if (buffer == NULL || *bufferSize < 18)
	{
		return STATUS_NO_ROOM;
	}

	// Build command buffer
	buffer[0] = PICC_CMD_MF_READ;
	buffer[1] = blockAddr;
	// Calculate CRC_A
	result = PCD_CalculateCRC(this, buffer, 2, &buffer[2]);
	if (result != STATUS_OK)
	{
		return result;
	}

	// Transmit the buffer and receive the response, validate CRC_A.
	return PCD_TransceiveData(this, buffer, 4, buffer, bufferSize, NULL, 0, true);
}

RFID_StatusCode_t MIFARE_Write(RFID_Reader_t* this, uint8_t blockAddr, uint8_t *buffer, uint8_t bufferSize)
{
	RFID_StatusCode_t result;

	// Sanity check
	if (buffer == NULL || bufferSize < 16) {
		return STATUS_INVALID;
	}

	/* Clean buffer */
	memset(mifareWriteCmddBuffer,0,MIFARE_WRITE_CMD_BUFFER_SIZE);

	// Mifare Classic protocol requires two communications to perform a write.
	// Step 1: Tell the PICC we want to write to block blockAddr.
	mifareWriteCmddBuffer[0] = PICC_CMD_MF_WRITE;
	mifareWriteCmddBuffer[1] = blockAddr;
	result = PCD_MIFARE_Transceive(this, mifareWriteCmddBuffer, MIFARE_WRITE_CMD_BUFFER_SIZE, false); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != STATUS_OK) {
		return result;
	}

	// Step 2: Transfer the data
	result = PCD_MIFARE_Transceive(this, buffer, bufferSize, false); // Adds CRC_A and checks that the response is MF_ACK.
	if (result != STATUS_OK) {
		return result;
	}

	return STATUS_OK;
}

RFID_StatusCode_t PCD_MIFARE_Transceive(RFID_Reader_t* this, uint8_t *sendData, uint8_t sendLen, bool acceptTimeout)
{
	RFID_StatusCode_t result;

	// Sanity check
	if (sendData == NULL || sendLen > 16) {
		return STATUS_INVALID;
	}

	/* Clean buffer */
	memset(mifareTransceivecmdBuffer,0,MIFARE_TRANSCEIVE_CMD_BUFFER_SIZE);

	// Copy sendData[] to cmdBuffer[] and add CRC_A
	memcpy(mifareTransceivecmdBuffer, sendData, sendLen);
	result = PCD_CalculateCRC(this, mifareTransceivecmdBuffer, sendLen, &mifareTransceivecmdBuffer[sendLen]);
	if (result != STATUS_OK)
	{
		return result;
	}
	sendLen += 2;

	// Transceive the data, store the reply in cmdBuffer[]
	uint8_t waitIRq = 0x30;		// RxIRq and IdleIRq
	uint8_t cmdBufferSize = MIFARE_TRANSCEIVE_CMD_BUFFER_SIZE;
	uint8_t validBits = 0;
	result = PCD_CommunicateWithPICC(this, PCD_Transceive, waitIRq, mifareTransceivecmdBuffer, sendLen, mifareTransceivecmdBuffer, &cmdBufferSize, &validBits, 0, false);
	if (acceptTimeout && result == STATUS_TIMEOUT)
	{
		return STATUS_OK;
	}
	if (result != STATUS_OK)
	{
		return result;
	}
	// The PICC must reply with a 4 bit ACK
	if (cmdBufferSize != 1 || validBits != 4)
	{
		return STATUS_ERROR;
	}
	if (mifareTransceivecmdBuffer[0] != MF_ACK) {
		return STATUS_MIFARE_NACK;
	}
	return STATUS_OK;
}

RFID_PICC_Type PICC_GetType(RFID_Reader_t* this, uint8_t sak)
{
	// http://www.nxp.com/documents/application_note/AN10833.pdf
	// 3.2 Coding of Select Acknowledge (SAK)
	// ignore 8-bit (iso14443 starts with LSBit = bit 1)
	// fixes wrong type for manufacturer Infineon (http://nfc-tools.org/index.php?title=ISO14443A)
	sak &= 0x7F;
	switch (sak) {
		case 0x04:	return PICC_TYPE_NOT_COMPLETE;	// UID not complete
		case 0x09:	return PICC_TYPE_MIFARE_MINI;
		case 0x08:	return PICC_TYPE_MIFARE_1K;
		case 0x18:	return PICC_TYPE_MIFARE_4K;
		case 0x00:	return PICC_TYPE_MIFARE_UL;
		case 0x10:
		case 0x11:	return PICC_TYPE_MIFARE_PLUS;
		case 0x01:	return PICC_TYPE_TNP3XXX;
		case 0x20:	return PICC_TYPE_ISO_14443_4;
		case 0x40:	return PICC_TYPE_ISO_18092;
		default:	return PICC_TYPE_UNKNOWN;
	}
}

CubeeErrorCode_t PCD_DumpVersionToSerial(RFID_Reader_t* this)
{
	/* Clean dump buffer */
	memset(dumpBuffer,0,DUMP_BUFFER_SIZE);

	// Get the MFRC522 firmware version
	uint8_t v = PCD_ReadSingleByteRegister(this, VersionReg);
	sprintf(dumpBuffer,  "Firmware Version: %0x", v);

	// Lookup which version
	switch(v)
	{
		case 0x88:
			sprintf(dumpBuffer, "%s = (clone)", dumpBuffer);
			break;
		case 0x90:
			sprintf(dumpBuffer, "%s = v0.0", dumpBuffer);
			break;
		case 0x91:
			sprintf(dumpBuffer, "%s = v1.0", dumpBuffer);
			break;
		case 0x92:
			sprintf(dumpBuffer, "%s = v2.0", dumpBuffer);
			break;
		default:
			sprintf(dumpBuffer, "%s = (unknown)", dumpBuffer);
	}

	ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);
	// When 0x00 or 0xFF is returned, communication probably failed
	if ((v == 0x00) || (v == 0xFF))
	{
		ESP_LOGI(LOG_HEADER, "WARNING: Communication failure, is the MFRC522 properly connected?");
	}

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t PICC_DumpToSerial(RFID_Reader_t* this, RFID_PICC_Uid_t *uid)
{
	RFID_MIFARE_Key_t key;

	// Dump UID, SAK and Type
	PICC_DumpDetailsToSerial(this, uid);

	// Dump contents
	RFID_PICC_Type piccType = PICC_GetType(this, uid->sak);
	switch (piccType) {
		case PICC_TYPE_MIFARE_MINI:
		case PICC_TYPE_MIFARE_1K:
		case PICC_TYPE_MIFARE_4K:

			// All keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
			for (uint8_t i = 0; i < 6; i++) {
				key.keyByte[i] = 0xFF;
			}
			PICC_DumpMifareClassicToSerial(this, uid, piccType, &key);
			break;

		case PICC_TYPE_MIFARE_UL:
			PICC_DumpMifareUltralightToSerial(this);
			break;

		case PICC_TYPE_ISO_14443_4:
		case PICC_TYPE_MIFARE_DESFIRE:
		case PICC_TYPE_ISO_18092:
		case PICC_TYPE_MIFARE_PLUS:
		case PICC_TYPE_TNP3XXX:
			ESP_LOGI(LOG_HEADER, "Dumping memory contents not implemented for that PICC type.");
			break;

		case PICC_TYPE_UNKNOWN:
		case PICC_TYPE_NOT_COMPLETE:
		default:
			break; // No memory dump here
	}

	ESP_LOGI(LOG_HEADER,"");
	PICC_HaltA(this); // Already done if it was a MIFARE Classic PICC.

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t PICC_DumpDetailsToSerial(RFID_Reader_t* this, RFID_PICC_Uid_t *uid)
{
	ESP_LOGI(LOG_HEADER, "PICC_DumpDetailsToSerial");

	/* Clean dump buffer */
	memset(dumpBuffer,0,DUMP_BUFFER_SIZE);

	// UID
	sprintf(dumpBuffer, "Card UID: ");
	for (uint8_t i = 0; i < uid->size; i++)
	{
		sprintf(dumpBuffer, "%s%2d", dumpBuffer, uid->uidByte[i]);
	}
	ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);

	/* Clean dump buffer */
	memset(dumpBuffer,0,DUMP_BUFFER_SIZE);

	// SAK
	sprintf(dumpBuffer, "Card SAK: %2d", uid->sak);
	ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);

	/* Clean dump buffer */
	memset(dumpBuffer,0,DUMP_BUFFER_SIZE);

	// (suggested) PICC type
	RFID_PICC_Type piccType = PICC_GetType(this, uid->sak);
	sprintf(dumpBuffer, "PICC type: %s", PICC_GetTypeName(this, piccType));
	ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t PICC_DumpMifareClassicToSerial(RFID_Reader_t* this, RFID_PICC_Uid_t *uid, RFID_PICC_Type piccType, RFID_MIFARE_Key_t *key)
{
	uint8_t no_of_sectors = 0;
	switch (piccType) {
		case PICC_TYPE_MIFARE_MINI:
			// Has 5 sectors * 4 blocks/sector * 16 bytes/block = 320 bytes.
			no_of_sectors = 5;
			break;

		case PICC_TYPE_MIFARE_1K:
			// Has 16 sectors * 4 blocks/sector * 16 bytes/block = 1024 bytes.
			no_of_sectors = 16;
			break;

		case PICC_TYPE_MIFARE_4K:
			// Has (32 sectors * 4 blocks/sector + 8 sectors * 16 blocks/sector) * 16 bytes/block = 4096 bytes.
			no_of_sectors = 40;
			break;

		default: // Should not happen. Ignore.
			break;
	}

	// Dump sectors, highest address first.
	if (no_of_sectors) {
		ESP_LOGI(LOG_HEADER, "Sector Block   0  1  2  3   4  5  6  7   8  9 10 11  12 13 14 15  AccessBits");
		for (int8_t i = no_of_sectors - 1; i >= 0; i--) {
			PICC_DumpMifareClassicSectorToSerial(this, uid, key, i);
		}
	}
	PICC_HaltA(this); // Halt the PICC before stopping the encrypted session.
	PCD_StopCrypto1(this);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t PICC_DumpMifareClassicSectorToSerial(RFID_Reader_t* this, RFID_PICC_Uid_t *uid, RFID_MIFARE_Key_t *key, uint8_t sector)
{
	RFID_StatusCode_t status;
	uint8_t firstBlock;		// Address of lowest address to dump actually last block dumped)
	uint8_t no_of_blocks;		// Number of blocks in sector
	bool isSectorTrailer;	// Set to true while handling the "last" (ie highest address) in the sector.

	// The access bits are stored in a peculiar fashion.
	// There are four groups:
	//		g[3]	Access bits for the sector trailer, block 3 (for sectors 0-31) or block 15 (for sectors 32-39)
	//		g[2]	Access bits for block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39)
	//		g[1]	Access bits for block 1 (for sectors 0-31) or blocks 5-9 (for sectors 32-39)
	//		g[0]	Access bits for block 0 (for sectors 0-31) or blocks 0-4 (for sectors 32-39)
	// Each group has access bits [C1 C2 C3]. In this code C1 is MSB and C3 is LSB.
	// The four CX bits are stored together in a nible cx and an inverted nible cx_.
	uint8_t c1, c2, c3;		// Nibbles
	uint8_t c1_, c2_, c3_;		// Inverted nibbles
	uint8_t invertedError;		// True if one of the inverted nibbles did not match
	uint8_t g[4];				// Access bits for each of the four groups.
	uint8_t group;				// 0-3 - active group for access bits
	bool firstInGroup;		// True for the first block dumped in the group

	// Determine position and size of sector.
	if (sector < 32) { // Sectors 0..31 has 4 blocks each
		no_of_blocks = 4;
		firstBlock = sector * no_of_blocks;
	}
	else if (sector < 40) { // Sectors 32-39 has 16 blocks each
		no_of_blocks = 16;
		firstBlock = 128 + (sector - 32) * no_of_blocks;
	}
	else { // Illegal input, no MIFARE Classic PICC has more than 40 sectors.
		return CUBEE_ERROR_UNDEFINED;
	}

	// Dump blocks, highest address first.
	uint8_t byteCount;
	uint8_t buffer[18];
	uint8_t blockAddr;
	isSectorTrailer = true;
	invertedError = false;	// Avoid "unused variable" warning.
	for (int8_t blockOffset = no_of_blocks - 1; blockOffset >= 0; blockOffset--)
	{
		/* Clean dump buffer */
		memset(dumpBuffer,0,DUMP_BUFFER_SIZE);

		blockAddr = firstBlock + blockOffset;
		// Sector number - only on first line
		if (isSectorTrailer) {
			if(sector < 10) {
				sprintf(dumpBuffer,"   "); // Pad with spaces
			}
			else {
				sprintf(dumpBuffer,"  "); // Pad with spaces
			}
			sprintf(dumpBuffer,"%s%d   ",dumpBuffer,sector);
		}
		else {
			sprintf(dumpBuffer,"       ");
		}
		// Block number
		if(blockAddr < 10) {
			sprintf(dumpBuffer,"%s   ",dumpBuffer); // Pad with spaces
		}
		else {
			if(blockAddr < 100) {
				sprintf(dumpBuffer,"%s  ",dumpBuffer); // Pad with spaces
			}
			else {
				sprintf(dumpBuffer,"%s ",dumpBuffer); // Pad with spaces
			}
		}
		sprintf(dumpBuffer,"%s%d  ",dumpBuffer,blockAddr);

		// Establish encrypted communications before reading the first block
		if (isSectorTrailer) {
			status = PCD_Authenticate(this, PICC_CMD_MF_AUTH_KEY_A, firstBlock, key, uid);
			if (status != STATUS_OK) {
				sprintf(dumpBuffer,"%sPCD_Authenticate() failed: %s",dumpBuffer, GetStatusCodeName(this, status));
				ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);
				return CUBEE_ERROR_UNDEFINED;
			}
		}
		// Read block
		byteCount = sizeof(buffer);
		status = MIFARE_Read(this, blockAddr, buffer, &byteCount);
		if (status != STATUS_OK) {
			sprintf(dumpBuffer,"%sMIFARE_Read() failed: %s",dumpBuffer, GetStatusCodeName(this, status));
			ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);
			continue;
		}
		// Dump data
		for (uint8_t index = 0; index < 16; index++) {
			sprintf(dumpBuffer,"%s %2d",dumpBuffer, buffer[index]);

			if ((index % 4) == 3) {				sprintf(dumpBuffer,"%s ",dumpBuffer);
			}
		}
		// Parse sector trailer data
		if (isSectorTrailer) {
			c1  = buffer[7] >> 4;
			c2  = buffer[8] & 0xF;
			c3  = buffer[8] >> 4;
			c1_ = buffer[6] & 0xF;
			c2_ = buffer[6] >> 4;
			c3_ = buffer[7] & 0xF;
			invertedError = (c1 != (~c1_ & 0xF)) || (c2 != (~c2_ & 0xF)) || (c3 != (~c3_ & 0xF));
			g[0] = ((c1 & 1) << 2) | ((c2 & 1) << 1) | ((c3 & 1) << 0);
			g[1] = ((c1 & 2) << 1) | ((c2 & 2) << 0) | ((c3 & 2) >> 1);
			g[2] = ((c1 & 4) << 0) | ((c2 & 4) >> 1) | ((c3 & 4) >> 2);
			g[3] = ((c1 & 8) >> 1) | ((c2 & 8) >> 2) | ((c3 & 8) >> 3);
			isSectorTrailer = false;
		}

		// Which access group is this block in?
		if (no_of_blocks == 4) {
			group = blockOffset;
			firstInGroup = true;
		}
		else {
			group = blockOffset / 5;
			firstInGroup = (group == 3) || (group != (blockOffset + 1) / 5);
		}

		if (firstInGroup) {
			// Print access bits
			sprintf(dumpBuffer,"%s [ %d %d %d ] ",dumpBuffer, (g[group] >> 2) & 1, (g[group] >> 1) & 1, (g[group] >> 0) & 1);
			if (invertedError) {
				sprintf(dumpBuffer,"%s Inverted access bits did not match! ",dumpBuffer);
			}
		}

		if (group != 3 && (g[group] == 1 || g[group] == 6)) { // Not a sector trailer, a value block
			int32_t value = ((int32_t)buffer[3]<<24) | ((int32_t)buffer[2]<<16) | ((int32_t)buffer[1]<<8) | ((int32_t)buffer[0]);
			sprintf(dumpBuffer,"%s Value=%0x Adr=%0x",dumpBuffer, value, buffer[12]);
		}
		ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);
	}

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t PICC_DumpMifareUltralightToSerial(RFID_Reader_t* this) {
	RFID_StatusCode_t status;
	uint8_t byteCount;
	uint8_t buffer[18];
	uint8_t i;

	/* Clean dump buffer */
	memset(dumpBuffer,0,DUMP_BUFFER_SIZE);

	sprintf(dumpBuffer,"Page  0  1  2  3");
	// Try the mpages of the original Ultralight. Ultralight C has more pages.
	for (uint8_t page = 0; page < 16; page +=4) { // Read returns data for 4 pages at a time.
		// Read pages
		byteCount = sizeof(buffer);
		status = MIFARE_Read(this, page, buffer, &byteCount);
		if (status != STATUS_OK)
		{
			sprintf(dumpBuffer,"MIFARE_Read() failed: %s", GetStatusCodeName(this, status));
			ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);
			break;
		}
		// Dump data
		for (uint8_t offset = 0; offset < 4; offset++) {
			i = page + offset;
			sprintf(dumpBuffer," %2d ", i);

			for (uint8_t index = 0; index < 4; index++) {
				i = 4 * offset + index;
				sprintf(dumpBuffer,"%s %2d ", dumpBuffer, buffer[i]);
			}
			ESP_LOGI(LOG_HEADER, "%s", dumpBuffer);
		}
	}

	return CUBEE_ERROR_OK;
}

bool PICC_IsNewCardPresent(RFID_Reader_t* this)
{
	uint8_t bufferATQA[2];
	uint8_t bufferSize = 2;
	RFID_StatusCode_t result;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER looking for new card");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Reset baud rates */
	if(PCD_WriteSingleByteRegister(this, TxModeReg, 0x00) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error looking for new card, Reset Rx baud rate failed");
		return CUBEE_ERROR_UNDEFINED;
	}
	if(PCD_WriteSingleByteRegister(this, RxModeReg, 0x00) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error looking for new card, Reset Tx baud rate failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Reset ModWidthReg */
	if(PCD_WriteSingleByteRegister(this, ModWidthReg, 0x26) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error looking for new card, ModWidthReg reset failed");
		return CUBEE_ERROR_UNDEFINED;
	}


	result = PICC_RequestA(this, bufferATQA, &bufferSize);
	return (result == STATUS_OK || result == STATUS_COLLISION);
}

bool PICC_ReadCardSerial(RFID_Reader_t* this) {
	RFID_StatusCode_t result = PICC_Select(this, &this->uid, 0);
	return (result == STATUS_OK);
}


char* PICC_GetTypeName(RFID_Reader_t* this, RFID_PICC_Type type)
{
	switch (type) {
		case PICC_TYPE_ISO_14443_4:		return "PICC compliant with ISO/IEC 14443-4";
		case PICC_TYPE_ISO_18092:		return "PICC compliant with ISO/IEC 18092 (NFC)";
		case PICC_TYPE_MIFARE_MINI:		return "MIFARE Mini, 320 bytes";
		case PICC_TYPE_MIFARE_1K:		return "MIFARE 1KB";
		case PICC_TYPE_MIFARE_4K:		return "MIFARE 4KB";
		case PICC_TYPE_MIFARE_UL:		return "MIFARE Ultralight or Ultralight C";
		case PICC_TYPE_MIFARE_PLUS:		return "MIFARE Plus";
		case PICC_TYPE_MIFARE_DESFIRE:	return "MIFARE DESFire";
		case PICC_TYPE_TNP3XXX:			return "MIFARE TNP3XXX";
		case PICC_TYPE_NOT_COMPLETE:	return "SAK indicates UID is not complete.";
		case PICC_TYPE_UNKNOWN:
		default:						return "Unknown type";
	}
}

char* GetStatusCodeName(RFID_Reader_t* this, RFID_StatusCode_t code)
{
	switch (code) {
		case STATUS_OK:				return "Success.";
		case STATUS_ERROR:			return "Error in communication.";
		case STATUS_COLLISION:		return "Collission detected.";
		case STATUS_TIMEOUT:		return "Timeout in communication.";
		case STATUS_NO_ROOM:		return "A buffer is not big enough.";
		case STATUS_INTERNAL_ERROR:	return "Internal error in the code. Should not happen.";
		case STATUS_INVALID:		return "Invalid argument.";
		case STATUS_CRC_WRONG:		return "The CRC_A does not match.";
		case STATUS_MIFARE_NACK:	return "A MIFARE PICC responded with NAK.";
		default:					return "Unknown error";
	}
}

/************************************************************************************************/
/******************************** Internal Functions Implementation  ****************************/
/************************************************************************************************/

#endif /* USING_RFID */
