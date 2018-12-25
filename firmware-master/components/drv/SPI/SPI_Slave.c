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
#include "driver/spi_slave.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/gpio_reg.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "SPI_Slave.h"

#define LOG_HEADER 	"[SPI_SLAVE DRV] "
#define DEBUG_SPI_SLAVE

#define GPIO_HANDSHAKE 2

/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/
//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
void SPI_Slave_post_setup_cb(spi_slave_transaction_t *trans);

//Called after transaction is sent/received. We use this to set the handshake line low.
void SPI_Slave_post_trans_cb(spi_slave_transaction_t *trans);

/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/
static SPI_Slave_t slaveInstances[SPI_SLAVE_ID_MAX] =
{
	[SPI_SLAVE_ID_RFID]	=
	{
		.initialized 	= false,
		.slaveId 		= SPI_SLAVE_ID_RFID,
		.slaveSem = NULL,
		.buscfg 		=
		{
			.mosi_io_num     = SPI_CFG_MOSI_PIN,		/*!< Bus OUTPUT pin */
			.miso_io_num     = SPI_CFG_MISO_PIN,		/*!< Bus INPUT pin */
			.sclk_io_num	 = SPI_CFG_CLK_PIN,			/*!< Clock signal */
		},
		.slvcfg =
		{
	        .mode 			= 0,
	        .spics_io_num 	= SPI_CFG_RFID_CS_PIN,
	        .queue_size 	= 1,
	        .flags 			= 0,
	        .post_setup_cb 	= SPI_Slave_post_setup_cb,
	        .post_trans_cb	= SPI_Slave_post_trans_cb,
		},
	},
};

/************************************************************************************************/
/* Public API  Implementation */
/************************************************************************************************/

CubeeErrorCode_t SPI_Slave_init(SPI_Slave_t* this)
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

    /* Enable pull-ups on SPI lines so we don't detect rogue pulses when no master is connected.*/
    gpio_set_pull_mode(SPI_CFG_MOSI_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(SPI_CFG_CLK_PIN, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(this->slvcfg.spics_io_num, GPIO_PULLUP_ONLY);

    /* Initialize SPI slave interface */
    if(spi_slave_initialize(HSPI_HOST, &this->buscfg, &this->slvcfg, SPI_CFG_DMA_CHANNEL) != ESP_OK)
    {
    	ESP_LOGE(LOG_HEADER,"Error initializing slave %d", this->slaveId);
    }

	/* Semaphore to synchronize  master and slave */
	this->slaveSem=xSemaphoreCreateBinary();

    this->initialized = true;

    ESP_LOGI(LOG_HEADER,"SPI slave driver successfully initialized");

	return CUBEE_ERROR_OK;
}

SPI_Slave_t * SPI_Slave_getInstance(SPI_Slave_Id_t slaveId)
{
	if(slaveId >= SPI_SLAVE_ID_MAX)
	{
		return NULL;
	}

	return  &slaveInstances[slaveId];
}

bool SPI_Slave_isInitialized(SPI_Slave_t* this)
{
	return  this->initialized;
}

/************************************************************************************************/
/* Internal Functions Implementation  */
/************************************************************************************************/

//Called after a transaction is queued and ready for pickup by master. We use this to set the handshake line high.
void SPI_Slave_post_setup_cb(spi_slave_transaction_t *trans)
{
	gpio_set_level(GPIO_HANDSHAKE,1);
}

//Called after transaction is sent/received. We use this to set the handshake line low.
void SPI_Slave_post_trans_cb(spi_slave_transaction_t *trans)
{
	gpio_set_level(GPIO_HANDSHAKE,0);
	xSemaphoreGiveFromISR(slaveInstances[SPI_SLAVE_ID_RFID].slaveSem,false);
}

