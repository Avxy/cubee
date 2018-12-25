/* ESP-IDF Libraries */
#include "esp_log.h"
#include "esp_err.h"
#include "driver/spi_master.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "SPI_Master_Cfg.h"

#define LOG_HEADER 	"[SPI_CFG ] "

/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/
static SPI_Cfg_t SpiCfgInstance =
{
		.initialized = false,
		.spiHost = HSPI_HOST,
		.bus_config =
		{
				.mosi_io_num     = SPI_CFG_MOSI_PIN,		/*!< Bus OUTPUT pin */
				.miso_io_num     = SPI_CFG_MISO_PIN,		/*!< Bus INPUT pin */
				.sclk_io_num	 = SPI_CFG_CLK_PIN,			/*!< Clock signal */
				.quadwp_io_num   = SPI_CFG_PIN_NOT_SET,		/*!< not used */
			    .quadhd_io_num   = SPI_CFG_PIN_NOT_SET,		/*!< not used */
				.max_transfer_sz = 0,       				/*!< 0 means use default. */
		},
		.slaveCfg =
		{
			[SPI_CFG_SLAVE_ID_RFID] =
			{
					.dev_config =
					{
								.clock_speed_hz   = SPI_CFG_RFID_CLK,		/*!< RFID data transfer frequency */
								.spics_io_num     = SPI_CFG_RFID_CS_PIN,	/*!< RFID chip selection pin */
					},
			},

		},
};


/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/


/************************************************************************************************/
/* Public API  Implementation */
/************************************************************************************************/
CubeeErrorCode_t SPI_Cfg_init(SPI_Cfg_t* this)
{
	uint8_t slaveId;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER initializing SPI configuration");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(this->initialized != false)
	{
		/* Already initialized */
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing SPI configuration...");

	/* Initializes the SPI bus */
	if(spi_bus_initialize(this->spiHost, &this->bus_config,SPI_CFG_DMA_CHANNEL) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "SPI bus initialization failure");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Initializes SPI slaves */
	for(slaveId = 0; slaveId < SPI_CFG_SLAVE_ID_MAX; ++slaveId)
	{
		this->slaveCfg[slaveId].dev_config.address_bits     = 0;
		this->slaveCfg[slaveId].dev_config.command_bits     = 0;
		this->slaveCfg[slaveId].dev_config.dummy_bits       = 0;
		this->slaveCfg[slaveId].dev_config.mode             = 0;
		this->slaveCfg[slaveId].dev_config.duty_cycle_pos   = 0;
		this->slaveCfg[slaveId].dev_config.cs_ena_posttrans = 0;
		this->slaveCfg[slaveId].dev_config.cs_ena_pretrans  = 0;
		this->slaveCfg[slaveId].dev_config.flags            = 0;
		this->slaveCfg[slaveId].dev_config.queue_size       = 1;
		this->slaveCfg[slaveId].dev_config.pre_cb           = NULL;
		this->slaveCfg[slaveId].dev_config.post_cb          = NULL;

		/* Adding device to SPI bus */
		if(spi_bus_add_device(this->spiHost, &this->slaveCfg[slaveId].dev_config, &this->slaveCfg[slaveId].deviceHandle) != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER, "SPI device configuration failure, device id: %d", slaveId);
			return CUBEE_ERROR_UNDEFINED;
		}
	}

	this->initialized = true;
	ESP_LOGI(LOG_HEADER,"SPI configuration successfully initialized");

	return  CUBEE_ERROR_OK;
}

SPI_Cfg_t * SPI_Cfg_getInstance()
{
	return  &SpiCfgInstance;
}

