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

#ifndef SPI_H_
#define SPI_H_

/* Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* ESP-IDF Libraries */
#include "driver/spi_master.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "SPI_Master_Cfg.h"

/************************************************************************************************/
/* TypeDefs */
/************************************************************************************************/

/*! Enum to index the SPI devices */
typedef enum
{
	SPI_SLAVE_ID_RFID = 0,							/*!< RFID device */

	SPI_SLAVE_ID_MAX,								/*!< Number of SPI slaves supported*/

} SPI_Slave_Id_t;


/*! Struct to instantiate the SPI component */
typedef struct
{
	bool initialized;								/*!< Indicates if the SPI component was initialized */
	SPI_Cfg_t *SpiCfg;								/*!< Pointer to the configurations used by the SPI instance */
	uint8_t * dataTransferBuffer;					/*!< Pointer to buffer used send data on SPI data transfer */
	uint16_t dataTransferBufferSize;				/*!< Size of data transfer buffer */
	uint8_t * dataReceiveBuffer;					/*!< Pointer to buffer used to receive data on SPI data transfer */
} SPI_t;

/************************************************************************************************/
/* Public API */
/************************************************************************************************/

/**
 * @brief This function initializes the SPI instance and all its dependencies.
 *
 * @param this - pointer to SPI instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the component SPI initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t SPI_init(SPI_t* this);

/**
* @brief This function gets the SPI configuration instance.
*
* @param this - pointer to SPI instance.
*
* @return pointer to the SPI configuration instance.
*/
SPI_Cfg_t * SPI_getConfig(SPI_t* this);

/**
* @brief This function gets the SPI instance.
*
* @return pointer to SPI instance.
*/
SPI_t * SPI_getInstance();

/**
 * @brief This function checks if the SPI instance is already initialized.
 *
 * @param this - pointer to SPI instance.
 *
 * @return
 * @arg true, if the SPI instance is initialized.
 * @arg false, otherwise.
*/
bool SPI_isInitialized(SPI_t* this);

/**
 * @brief This function transfer the data stored in the dataTransferBuffer to the SPI slave specified.
 * The data size needs to be defined on dataTransferBufferSize.
 *
 * @param this - pointer to SPI instance.
 * @param slaveId - specifies the SPI slave device
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully transfered to the SPI slave.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if the parameters used to data transfer isn't correct.
 * @arg CUBEE_ERROR_UNDEFINED, if the SPI data transfer failed.
*/
CubeeErrorCode_t SPI_transferData(SPI_t* this, SPI_Slave_Id_t slaveId);


#endif /* SPI_H_ */
