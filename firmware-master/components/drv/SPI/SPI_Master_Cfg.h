#ifndef SPI_CFG_H_
#define SPI_CFG_H_

/* Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* ESP-IDF Libraries */
#include "driver/spi_master.h"
#include "driver/gpio.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define SPI_CFG_MOSI_PIN 		GPIO_NUM_13		/*!< MOSI pin */
#define SPI_CFG_MISO_PIN 		GPIO_NUM_12		/*!< MISO pin */
#define SPI_CFG_CLK_PIN  		GPIO_NUM_14		/*!< CLOCK pin */
#define SPI_CFG_RFID_CS_PIN		GPIO_NUM_15		/*!< Chip select pin used to RFID slave */
//#define SPI_CFG_MOSI_PIN 		GPIO_NUM_23		/*!< MOSI pin */
//#define SPI_CFG_MISO_PIN 		GPIO_NUM_19		/*!< MISO pin */
//#define SPI_CFG_CLK_PIN  		GPIO_NUM_18		/*!< CLOCK pin */
//#define SPI_CFG_RFID_CS_PIN		GPIO_NUM_21		/*!< Chip select pin used to RFID slave */

#define SPI_CFG_PIN_NOT_SET     (-1)			/*!< Configuration to SPI pins not used */
#define SPI_CFG_DMA_CHANNEL		(1)				/*!< DMA channel used to SPI data transfer */
#define SPI_CFG_RFID_CLK		(100000)		/*!< Clock used to RFID data transfer - 100 kHz */

/*! Enum to index the SPI devices configuration */
typedef enum
{
	SPI_CFG_SLAVE_ID_RFID = 0,							/*!< RFID device configuration*/

	SPI_CFG_SLAVE_ID_MAX,								/*!< Number of slave configuration supported*/
} SPI_Cfg_Slave_Id_t;

/*! Struct to configure a slave device and add it to the SPI bus*/
typedef struct
{
	spi_device_interface_config_t dev_config;			/*!< SPI slave device configuration */
	spi_device_handle_t deviceHandle;					/*!< Handle to device added to SPI bus */
}SPI_Slave_Cfg_t;

/*! Struct to instatiate a SPI configuration*/
typedef struct
{
	bool initialized;
	spi_host_device_t   spiHost;					/*!< SPI peripheral used */
	spi_bus_config_t bus_config;					/*!< SPI bus configuration*/
	SPI_Slave_Cfg_t slaveCfg[SPI_CFG_SLAVE_ID_MAX]; /*!< Table with configuration of the SPI slaves connected on the SPI bus */
} SPI_Cfg_t;

/**
 * @brief This function initializes the SPI configuration instance and all its dependencies.
 *
 * @param this - pointer to SPI configuration instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t SPI_Cfg_init(SPI_Cfg_t* this);

/**
* @brief This function gets the SPI configuration instance.
*
* @return pointer to SPI configuration instance.
*/
SPI_Cfg_t* SPI_Cfg_getInstance();



#endif /* SPI_CFG_H_ */
