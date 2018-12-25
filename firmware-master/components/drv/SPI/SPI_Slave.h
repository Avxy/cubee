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

#ifndef SPI_SLAVE_H_
#define SPI_SLAVE_H_

/* Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* FreeRTOS libraries */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* ESP-IDF Libraries */
#include "driver/spi_slave.h"
#include "driver/gpio.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "SPI_Master.h"
#include "SPI_Master_Cfg.h"

/************************************************************************************************/
/* TypeDefs */
/************************************************************************************************/

/*! Struct to instantiate the SPI slave component */
typedef struct
{
	bool initialized;												/*!< Indicates if the SPI component was initialized */
	spi_bus_config_t buscfg;										/*!< SPI bus configuration */
	spi_slave_interface_config_t slvcfg;							/*!< SPI Slave configuration */
	SPI_Slave_Id_t slaveId;											/*!< SPI Slave Id */
	spi_slave_transaction_t	transaction;							/*!< Transaction to send/receive data */
	xQueueHandle slaveSem;											/*!< Semaphore to synchronize master/slave communication */
} SPI_Slave_t;

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
CubeeErrorCode_t SPI_Slave_init(SPI_Slave_t* this);

/**
* @brief This function gets the SPI slave instance.
*
* @return pointer to SPI instance.
*/
SPI_Slave_t * SPI_Slave_getInstance(SPI_Slave_Id_t slaveId);

/**
 * @brief This function checks if the SPI instance is already initialized.
 *
 * @param this - pointer to SPI instance.
 *
 * @return
 * @arg true, if the SPI instance is initialized.
 * @arg false, otherwise.
*/
bool SPI_Slave_isInitialized(SPI_Slave_t* this);


#endif /* SPI_SLAVE_H_ */
