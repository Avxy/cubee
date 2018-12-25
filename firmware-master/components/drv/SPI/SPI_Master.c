/**
 * @file SPI.h
 * @version 1.1
 * @author Alex Fernandes
 * @date September 01, 2017
 **************************************************************************
 *
 * @brief  Driver SPI
 * ADD DESCRIPTION
 *
 * @section References
 * @ref 1. Kolban - SPI driver
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   01-Sep-2017    Alex Fernandes
 * * Original version based on Kolban SPI driver
 *
 ******************************************************************/

/* ESP-IDF Libraries */
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "driver/spi_master.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "SPI_Master.h"

#define LOG_HEADER 	"[SPI DRV] "
#define xDEBUG_SPI

/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/
SPI_t spiInstance =
{
	.initialized = false,
	.SpiCfg = NULL,
	.dataTransferBuffer = NULL,
	.dataTransferBufferSize = 0,
};


/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/


/************************************************************************************************/
/* Public API  Implementation */
/************************************************************************************************/

CubeeErrorCode_t SPI_init(SPI_t* this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER initializing SPI driver");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(this->initialized != false)
	{
		/* Already initialized */
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing SPI driver...");

	this->SpiCfg = SPI_Cfg_getInstance();
	if(SPI_Cfg_init(this->SpiCfg) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing SPI configuration");
		return CUBEE_ERROR_UNDEFINED;
	}
	this->initialized = true;

	ESP_LOGI(LOG_HEADER,"SPI driver successfully initialized");

	return CUBEE_ERROR_OK;
}

SPI_Cfg_t * SPI_getConfig(SPI_t* this)
{
	return  this->SpiCfg;
}

SPI_t * SPI_getInstance()
{
	return  &spiInstance;
}

bool SPI_isInitialized(SPI_t* this)
{
	return  this->initialized;
}


CubeeErrorCode_t SPI_transferData(SPI_t* this, SPI_Slave_Id_t slaveId)
{
	spi_transaction_t trans_desc;

	if((this == NULL) || (this->dataTransferBuffer == NULL) || (this->dataTransferBufferSize == 0))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER transferring SPI data");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
#ifdef DEBUG_SPI
	ESP_LOGI(LOG_HEADER, "SPI data transfer:");
	for (uint16_t byteIndex=0; byteIndex < this->dataTransferBufferSize; byteIndex++)
	{
		ESP_LOGI(LOG_HEADER, "> %2d %.2x", byteIndex, this->dataTransferBuffer[byteIndex]);
	}
#endif

	/* Make transition */
	trans_desc.flags     = 0;
	trans_desc.length    = (this->dataTransferBufferSize) * 8;
	trans_desc.rxlength  = 0;
	trans_desc.tx_buffer = this->dataTransferBuffer;
	trans_desc.rx_buffer = this->dataReceiveBuffer;

	/* Data transmission*/
	if (spi_device_transmit(this->SpiCfg->slaveCfg[slaveId].deviceHandle, &trans_desc) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error transmitting data to SPI slave %d", slaveId);
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

/************************************************************************************************/
/* Internal Functions Implementation  */
/************************************************************************************************/


