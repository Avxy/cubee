/**
 * MFRC522.h - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY COOQROBOT.
 * Based on code Dr.Leong   ( WWW.B2CQSHOP.COM )
 * Created by Miguel Balboa (circuitito.com), Jan, 2012.
 * Rewritten by Søren Thing Andersen (access.thing.dk), fall of 2013 (Translation to English, refactored, comments, anti collision, cascade levels.)
 * Extended by Tom Clement with functionality to write to sector 0 of RFID_PICC_Uid_t changeable Mifare cards.
 * Released into the public domain.
 *
 * Please read this file for an overview and then MFRC522.cpp for comments on the specific functions.
 * Search for "mf-rc522" on ebay.com to purchase the MF-RC522 board.
 *
 * There are three hardware components involved:
 * 1) The micro controller: An Arduino
 * 2) The PCD (short for Proximity Coupling Device): NXP MFRC522 Contactless Reader IC
 * 3) The PICC (short for Proximity Integrated Circuit Card): A card or tag using the ISO 14443A interface, eg Mifare or NTAG203.
 *
 * The microcontroller and card reader uses SPI for communication.
 * The protocol is described in the MFRC522 datasheet: http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 *
 * The card reader and the tags communicate using a 13.56MHz electromagnetic field.
 * The protocol is defined in ISO/IEC 14443-3 Identification cards -- Contactless integrated circuit cards -- Proximity cards -- Part 3: Initialization and anticollision".
 * A free version of the final draft can be found at http://wg8.de/wg8n1496_17n3613_Ballot_FCD14443-3.pdf
 * Details are found in chapter 6, Type A – Initialization and anticollision.
 *
 * If only the PICC RFID_PICC_Uid_t is wanted, the above documents has all the needed information.
 * To read and write from MIFARE PICCs, the MIFARE protocol is used after the PICC has been selected.
 * The MIFARE Classic chips and protocol is described in the datasheets:
 *		1K:   http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf
 * 		4K:   http://datasheet.octopart.com/MF1S7035DA4,118-NXP-Semiconductors-datasheet-11046188.pdf
 * 		Mini: http://www.idcardmarket.com/download/mifare_S20_datasheet.pdf
 * The MIFARE Ultralight chip and protocol is described in the datasheets:
 *		Ultralight:   http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf
 * 		Ultralight C: http://www.nxp.com/documents/short_data_sheet/MF0ICU2_SDS.pdf
 *
 * MIFARE Classic 1K (MF1S503x):
 * 		Has 16 sectors * 4 blocks/sector * 16 bytes/block = 1024 bytes.
 * 		The blocks are numbered 0-63.
 * 		Block 3 in each sector is the Sector Trailer. See http://www.mouser.com/ds/2/302/MF1S503x-89574.pdf sections 8.6 and 8.7:
 * 				Bytes 0-5:   Key A
 * 				Bytes 6-8:   Access Bits
 * 				Bytes 9:     User data
 * 				Bytes 10-15: Key B (or user data)
 * 		Block 0 is read-only manufacturer data.
 * 		To access a block, an authentication using a key from the block's sector must be performed first.
 * 		Example: To read from block 10, first authenticate using a key from sector 3 (blocks 8-11).
 * 		All keys are set to FFFFFFFFFFFFh at chip delivery.
 * 		Warning: Please read section 8.7 "Memory Access". It includes this text: if the PICC detects a format violation the whole sector is irreversibly blocked.
 *		To use a block in "value block" mode (for Increment/Decrement operations) you need to change the sector trailer. Use PICC_SetAccessBits() to calculate the bit patterns.
 * MIFARE Classic 4K (MF1S703x):
 * 		Has (32 sectors * 4 blocks/sector + 8 sectors * 16 blocks/sector) * 16 bytes/block = 4096 bytes.
 * 		The blocks are numbered 0-255.
 * 		The last block in each sector is the Sector Trailer like above.
 * MIFARE Classic Mini (MF1 IC S20):
 * 		Has 5 sectors * 4 blocks/sector * 16 bytes/block = 320 bytes.
 * 		The blocks are numbered 0-19.
 * 		The last block in each sector is the Sector Trailer like above.
 *
 * MIFARE Ultralight (MF0ICU1):
 * 		Has 16 pages of 4 bytes = 64 bytes.
 * 		Pages 0 + 1 is used for the 7-byte RFID_PICC_Uid_t.
 * 		Page 2 contains the last check digit for the RFID_PICC_Uid_t, one byte manufacturer internal data, and the lock bytes (see http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf section 8.5.2)
 * 		Page 3 is OTP, One Time Programmable bits. Once set to 1 they cannot revert to 0.
 * 		Pages 4-15 are read/write unless blocked by the lock bytes in page 2.
 * MIFARE Ultralight C (MF0ICU2):
 * 		Has 48 pages of 4 bytes = 192 bytes.
 * 		Pages 0 + 1 is used for the 7-byte RFID_PICC_Uid_t.
 * 		Page 2 contains the last check digit for the RFID_PICC_Uid_t, one byte manufacturer internal data, and the lock bytes (see http://www.nxp.com/documents/data_sheet/MF0ICU1.pdf section 8.5.2)
 * 		Page 3 is OTP, One Time Programmable bits. Once set to 1 they cannot revert to 0.
 * 		Pages 4-39 are read/write unless blocked by the lock bytes in page 2.
 * 		Page 40 Lock bytes
 * 		Page 41 16 bit one way counter
 * 		Pages 42-43 Authentication configuration
 * 		Pages 44-47 Authentication key
 */

#ifndef RFID_READER_H_
#define RFID_READER_H_

/* Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* Modules dependencies */
#include "RFID_Reader_Cfg.h"
#include "../../drv/SPI/SPI_Master.h"
#include "../../drv/SPI/SPI_Master_Cfg.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define RFID_WRITE_REGISTER_BUFF_MAX_SIZE	(256)

/************************************************************************************************/
/* TypeDefs */
/************************************************************************************************/

typedef struct
{
	bool initialized;
	RFID_Reader_Cfg_t *rfidReaderCfg;
	SPI_t *spi;
	uint8_t writeRegisterBuffer[RFID_WRITE_REGISTER_BUFF_MAX_SIZE];			/*!< Buffer used to write register bytes */
	uint8_t _chipSelectPin;													/*!< Pin connected to MFRC522's SPI slave select input (Pin 24, NSS, active low) */
	uint8_t _resetPowerDownPin;												/*!< Arduino pin connected to MFRC522's reset and power down input (Pin 6, NRSTPD, active low) */
	RFID_PICC_Uid_t uid;										/*!< PICC URFID_PICC_Uid_t Used by PICC_ReadCardSerial().*/
} RFID_Reader_t;

/************************************************************************************************/
/* Public API */
/************************************************************************************************/

/**
 * @brief This function initializes the RFID instance and all its dependencies.
 *
 * @param this - pointer to RFID instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the RFID initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t RFID_Reader_init(RFID_Reader_t* this);

/**
* @brief This function gets the RFID instance.
*
* @return pointer to RFID instance.
*/
RFID_Reader_Cfg_t * RFID_Reader_getConfig(RFID_Reader_t* this);

/**
* @brief This function gets the RFID instance.
*
* @return pointer to RFID instance.
*/
RFID_Reader_t * RFID_Reader_getInstance();

/**
 * @brief This function checks if the RFID instance is already initialized.
 *
 * @param this - pointer to RFID instance.
 *
 * @return
 * @arg true, if the RFID instance is initialized.
 * @arg false, otherwise.
*/
bool RFID_Reader_isInitialized(RFID_Reader_t* this);


/////////////////////////////////////////////////////////////////////////////////////
// Basic interface functions for communicating with the MFRC522
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Writes a byte to the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
CubeeErrorCode_t PCD_WriteSingleByteRegister(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t value);

/**
 * Writes a number of bytes to the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
CubeeErrorCode_t PCD_WriteMultipleByteRegister(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t count, uint8_t *values);

/**
 * Reads a byte from the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
uint8_t PCD_ReadSingleByteRegister(RFID_Reader_t* this, RFID_PCD_Register_t reg);

/**
 * Reads a number of bytes from the specified register in the MFRC522 chip.
 * The interface is described in the datasheet section 8.1.2.
 */
CubeeErrorCode_t PCD_ReadMultipleByteRegister(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t count, uint8_t *values, uint8_t rxAlign);

/**
 * Sets the bits given in mask in register reg.
 */
CubeeErrorCode_t PCD_SetRegisterBitMask(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t mask);

/**
 * Clears the bits given in mask from register reg.
 */
CubeeErrorCode_t PCD_ClearRegisterBitMask(RFID_Reader_t* this, RFID_PCD_Register_t reg, uint8_t mask);

/**
 * Use the CRC coprocessor in the MFRC522 to calculate a CRC_A.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PCD_CalculateCRC(RFID_Reader_t* this, uint8_t *data, uint8_t length, uint8_t *result);


/////////////////////////////////////////////////////////////////////////////////////
// Functions for manipulating the MFRC522
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Initializes the MFRC522 chip.
 */
CubeeErrorCode_t PCD_Init(RFID_Reader_t* this);


/**
 * Performs a soft reset on the MFRC522 chip and waits for it to be ready again.
 */
CubeeErrorCode_t PCD_Reset(RFID_Reader_t* this);

/**
 * Turns the antenna on by enabling pins TX1 and TX2.
 * After a reset these pins are disabled.
 */
CubeeErrorCode_t PCD_AntennaOn(RFID_Reader_t* this);

/**
 * Turns the antenna off by disabling pins TX1 and TX2.
 */
CubeeErrorCode_t PCD_AntennaOff(RFID_Reader_t* this);

/**
 * Get the current MFRC522 Receiver Gain (RxGain[2:0]) value.
 * See 9.3.3.6 / table 98 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * NOTE: Return value scrubbed with (0x07<<4)=01110000b as RCFfgReg may use reserved bits.
 *
 * @return Value of the RxGain, scrubbed to the 3 bits used.
 */
uint8_t PCD_GetAntennaGain(RFID_Reader_t* this);

/**
 * Set the MFRC522 Receiver Gain (RxGain) to value specified by given mask.
 * See 9.3.3.6 / table 98 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 * NOTE: Given mask is scrubbed with (0x07<<4)=01110000b as RCFfgReg may use reserved bits.
 */
void PCD_SetAntennaGain(RFID_Reader_t* this, uint8_t mask);

/**
 * Performs a self-test of the MFRC522
 * See 16.1.1 in http://www.nxp.com/documents/data_sheet/MFRC522.pdf
 *
 * @return Whether or not the test passed. Or false if no firmware reference is available.
 */
bool PCD_PerformSelfTest(RFID_Reader_t* this);

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with PICCs
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Executes the Transceive command.
 * CRC validation can only be done if backData and backLen are specified.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PCD_TransceiveData(RFID_Reader_t* this, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC);

/**
 * Transfers data to the MFRC522 FIFO, executes a command, waits for completion and transfers data back from the FIFO.
 * CRC validation can only be done if backData and backLen are specified.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PCD_CommunicateWithPICC(RFID_Reader_t* this, uint8_t command, uint8_t waitIRq, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen, uint8_t *validBits, uint8_t rxAlign, bool checkCRC);

/**
 * Transmits a REQuest command, Type A. Invites PICCs in state IDLE to go to READY and prepare for anticollision or selection. 7 bit frame.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PICC_RequestA(RFID_Reader_t* this, uint8_t *bufferATQA, uint8_t *bufferSize);

/**
 * Transmits a Wake-UP command, Type A. Invites PICCs in state IDLE and HALT to go to READY(*) and prepare for anticollision or selection. 7 bit frame.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PICC_WakeupA(RFID_Reader_t* this, uint8_t *bufferATQA, uint8_t *bufferSize);

/**
 * Transmits REQA or WUPA commands.
 * Beware: When two PICCs are in the field at the same time I often get STATUS_TIMEOUT - probably due do bad antenna design.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PICC_REQA_or_WUPA(RFID_Reader_t* this, uint8_t command, uint8_t *bufferATQA, uint8_t *bufferSize);

/**
 * Transmits SELECT/ANTICOLLISION commands to select a single PICC.
 * Before calling this function the PICCs must be placed in the READY(*) state by calling PICC_RequestA() or PICC_WakeupA().
 * On success:
 * 		- The chosen PICC is in state ACTIVE(*) and all other PICCs have returned to state IDLE/HALT. (Figure 7 of the ISO/IEC 14443-3 draft.)
 * 		- The RFID_PICC_Uid_t size and value of the chosen PICC is returned in *RFID_PICC_Uid_t along with the SAK.
 *
 * A PICC RFID_PICC_Uid_t consists of 4, 7 or 10 bytes.
 * Only 4 bytes can be specified in a SELECT command, so for the longer RFID_PICC_Uid_ts two or three iterations are used:
 * 		RFID_PICC_Uid_t size	Number of RFID_PICC_Uid_t bytes		Cascade levels		Example of PICC
 * 		========	===================		==============		===============
 * 		single				 4						1				MIFARE Classic
 * 		double				 7						2				MIFARE Ultralight
 * 		triple				10						3				Not currently in use?
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PICC_Select(RFID_Reader_t* this, RFID_PICC_Uid_t *uid, uint8_t validBits);

/**
 * Instructs a PICC in state ACTIVE(*) to go to state HALT.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PICC_HaltA(RFID_Reader_t* this);

/////////////////////////////////////////////////////////////////////////////////////
// Functions for communicating with MIFARE PICCs
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Executes the MFRC522 MFAuthent command.
 * This command manages MIFARE authentication to enable a secure communication to any MIFARE Mini, MIFARE 1K and MIFARE 4K card.
 * The authentication is described in the MFRC522 datasheet section 10.3.1.9 and http://www.nxp.com/documents/data_sheet/MF1S503x.pdf section 10.1.
 * For use with MIFARE Classic PICCs.
 * The PICC must be selected - ie in state ACTIVE(*) - before calling this function.
 * Remember to call PCD_StopCrypto1() after communicating with the authenticated PICC - otherwise no new communications can start.
 *
 * All keys are set to FFFFFFFFFFFFh at chip delivery.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise. Probably STATUS_TIMEOUT if you supply the wrong key.
 */
RFID_StatusCode_t PCD_Authenticate(RFID_Reader_t* this, uint8_t command, uint8_t blockAddr, RFID_MIFARE_Key_t *key, RFID_PICC_Uid_t *RFID_PICC_Uid_t);

/**
 * Used to exit the PCD from its authenticated state.
 * Remember to call this function after communicating with an authenticated PICC - otherwise no new communications can start.
 */
CubeeErrorCode_t PCD_StopCrypto1(RFID_Reader_t* this);

/**
 * Reads 16 bytes (+ 2 bytes CRC_A) from the active PICC.
 *
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 *
 * For MIFARE Ultralight only addresses 00h to 0Fh are decoded.
 * The MF0ICU1 returns a NAK for higher addresses.
 * The MF0ICU1 responds to the READ command by sending 16 bytes starting from the page address defined by the command argument.
 * For example; if blockAddr is 03h then pages 03h, 04h, 05h, 06h are returned.
 * A roll-back is implemented: If blockAddr is 0Eh, then the contents of pages 0Eh, 0Fh, 00h and 01h are returned.
 *
 * The buffer must be at least 18 bytes because a CRC_A is also returned.
 * Checks the CRC_A before returning STATUS_OK.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_Read(RFID_Reader_t* this, uint8_t blockAddr, uint8_t *buffer, uint8_t *bufferSize);

/**
 * Writes 16 bytes to the active PICC.
 *
 * For MIFARE Classic the sector containing the block must be authenticated before calling this function.
 *
 * For MIFARE Ultralight the operation is called "COMPATIBILITY WRITE".
 * Even though 16 bytes are transferred to the Ultralight PICC, only the least significant 4 bytes (bytes 0 to 3)
 * are written to the specified address. It is recommended to set the remaining bytes 04h to 0Fh to all logic 0.
 * *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_Write(RFID_Reader_t* this, uint8_t blockAddr, uint8_t *buffer, uint8_t bufferSize);

/**
 * Writes a 4 byte page to the active MIFARE Ultralight PICC.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_Ultralight_Write(RFID_Reader_t* this, uint8_t page, uint8_t *buffer, uint8_t bufferSize);

/**
 * MIFARE Decrement subtracts the delta from the value of the addressed block, and stores the result in a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_Decrement(RFID_Reader_t* this, uint8_t blockAddr, int32_t delta);

/**
 * MIFARE Increment adds the delta to the value of the addressed block, and stores the result in a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_Increment(RFID_Reader_t* this, uint8_t blockAddr, int32_t delta);

/**
 * MIFARE Restore copies the value of the addressed block into a volatile memory.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 * Use MIFARE_Transfer() to store the result in a block.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_Restore(RFID_Reader_t* this, uint8_t blockAddr);

/**
 * MIFARE Transfer writes the value stored in the volatile memory into one MIFARE Classic block.
 * For MIFARE Classic only. The sector containing the block must be authenticated before calling this function.
 * Only for blocks in "value block" mode, ie with access bits [C1 C2 C3] = [110] or [001].
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_Transfer(RFID_Reader_t* this, uint8_t blockAddr);

/**
 * Helper routine to read the current value from a Value Block.
 *
 * Only for MIFARE Classic and only for blocks in "value block" mode, that
 * is: with access bits [C1 C2 C3] = [110] or [001]. The sector containing
 * the block must be authenticated before calling this function.
 *
 * @param[in]   blockAddr   The block (0x00-0xff) number.
 * @param[out]  value       Current value of the Value Block.
 * @return STATUS_OK on success, STATUS_??? otherwise.
  */
RFID_StatusCode_t MIFARE_GetValue(RFID_Reader_t* this, uint8_t blockAddr, int32_t *value);

/**
 * Helper routine to write a specific value into a Value Block.
 *
 * Only for MIFARE Classic and only for blocks in "value block" mode, that
 * is: with access bits [C1 C2 C3] = [110] or [001]. The sector containing
 * the block must be authenticated before calling this function.
 *
 * @param[in]   blockAddr   The block (0x00-0xff) number.
 * @param[in]   value       New value of the Value Block.
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_SetValue(RFID_Reader_t* this, uint8_t blockAddr, int32_t value);

/**
 * Authenticate with a NTAG216.
 *
 * Only for NTAG216. First implemented by Gargantuanman.
 *
 * @param[in]   passWord   password.
 * @param[in]   pACK       result success???.
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PCD_NTAG216_AUTH(RFID_Reader_t* this, uint8_t *passWord, uint8_t pACK[]);


/////////////////////////////////////////////////////////////////////////////////////
// Support functions
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Wrapper for MIFARE protocol communication.
 * Adds CRC_A, executes the Transceive command and checks that the response is MF_ACK or a timeout.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t PCD_MIFARE_Transceive(RFID_Reader_t* this, uint8_t *sendData, uint8_t sendLen, bool acceptTimeout);

/**
 * Translates the SAK (Select Acknowledge) to a PICC type.
 *
 * @return RFID_PICC_Type
 */
RFID_PICC_Type PICC_GetType(RFID_Reader_t* this, uint8_t sak);

// Support functions for debuging

/**
 * Dumps debug info about the connected PCD to Serial.
 * Shows all known firmware versions
 */
CubeeErrorCode_t PCD_DumpVersionToSerial(RFID_Reader_t* this);

/**
 * Dumps debug info about the selected PICC to Serial.
 * On success the PICC is halted after dumping the data.
 * For MIFARE Classic the factory default key of 0xFFFFFFFFFFFF is tried.
 *
 * @DEPRECATED Kept for bakward compatibility
 */
CubeeErrorCode_t PICC_DumpToSerial(RFID_Reader_t* this, RFID_PICC_Uid_t *uid);

/**
 * Dumps card info (RFID_PICC_Uid_t,SAK,Type) about the selected PICC to Serial.
 *
 * @DEPRECATED kept for backward compatibility
 */
CubeeErrorCode_t PICC_DumpDetailsToSerial(RFID_Reader_t* this, RFID_PICC_Uid_t *uid);

/**
 * Dumps memory contents of a MIFARE Classic PICC.
 * On success the PICC is halted after dumping the data.
 */
CubeeErrorCode_t PICC_DumpMifareClassicToSerial(RFID_Reader_t* this, RFID_PICC_Uid_t *uid, RFID_PICC_Type piccType, RFID_MIFARE_Key_t *key);

/**
 * Dumps memory contents of a sector of a MIFARE Classic PICC.
 * Uses PCD_Authenticate(), MIFARE_Read() and PCD_StopCrypto1.
 * Always uses PICC_CMD_MF_AUTH_KEY_A because only Key A can always read the sector trailer access bits.
 */
CubeeErrorCode_t PICC_DumpMifareClassicSectorToSerial(RFID_Reader_t* this, RFID_PICC_Uid_t *RFID_PICC_Uid_t, RFID_MIFARE_Key_t *key, uint8_t sector);

/**
 * Dumps memory contents of a MIFARE Ultralight PICC.
 */
CubeeErrorCode_t PICC_DumpMifareUltralightToSerial(RFID_Reader_t* this);

// Advanced functions for MIFARE

/**
 * Calculates the bit pattern needed for the specified access bits. In the [C1 C2 C3] tuples C1 is MSB (=4) and C3 is LSB (=1).
 */
void MIFARE_SetAccessBits(RFID_Reader_t* this, uint8_t *accessBitBuffer, uint8_t g0, uint8_t g1, uint8_t g2, uint8_t g3);

/////////////////////////////////////////////////////////////////////////////////////
// Convenience functions - does not add extra functionality
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Returns true if a PICC responds to PICC_CMD_REQA.
 * Only "new" cards in state IDLE are invited. Sleeping cards in state HALT are ignored.
 *
 * @return bool
 */
bool PICC_IsNewCardPresent(RFID_Reader_t* this);

/**
 * Simple wrapper around PICC_Select.
 * Returns true if a RFID_PICC_Uid_t could be read.
 * Remember to call PICC_IsNewCardPresent(), PICC_RequestA() or PICC_WakeupA() first.
 * The read RFID_PICC_Uid_t is available in the class variable RFID_PICC_Uid_t.
 *
 * @return bool
 */
bool PICC_ReadCardSerial(RFID_Reader_t* this );

// Functions for communicating with MIFARE PICCs

/**
 * Helper function for the two-step MIFARE Classic protocol operations Decrement, Increment and Restore.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
RFID_StatusCode_t MIFARE_TwoStepHelper(RFID_Reader_t* this, uint8_t command, uint8_t blockAddr, int32_t data);


/////////////////////////////////////////////////////////////////////////////////////
// DEBUG - Get human readable code and type
/////////////////////////////////////////////////////////////////////////////////////
char* PICC_GetTypeName(RFID_Reader_t* this, RFID_PICC_Type type);

char* GetStatusCodeName(RFID_Reader_t* this, RFID_StatusCode_t code);

#endif /* RFID_READER_H_ */
