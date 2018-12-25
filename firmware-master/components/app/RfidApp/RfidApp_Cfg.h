/**
 * @file RfidApp_Cfg.h
 * @version 1.0
 * @author Alex Fernandes
 * @date June 06, 2017
 **************************************************************************
 *
 * @brief  Wifi Controller Configuration
 * This module has configurations used by the Wifi Controller.
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   06-Jun-2017    Alex Fernandes
 * * Original version
 ******************************************************************/

#ifndef RFID_APP_CFG_H_
#define RFID_APP_CFG_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* Modules dependencies */
#include "../../../main/ProjectCfg.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define RFID_APP_SIGNAL_QUEUE_LENGTH		(5)										/*!< Length of queue used to store RFID APP state machine signals */
#define RFID_APP_SIGNAL_QUEUE_ITEM_SIZE   	sizeof(RfidApp_Signals_t )				/*!< Size of RFID APP controller state machine signals */
#define RFID_APP_SIGNAL_QUEUE_TIMEOUT		(10 / portTICK_PERIOD_MS)				/*!< Waiting timeout to receive a signal from the RFID APP state machine queue */
#define RFID_APP_SEMAPHORE_TIMEOUT			1000 / portTICK_PERIOD_MS				/*!< Waiting timeout to access RFID APP shared resources */
#define RFID_APP_BLOCK_ADDRESS_USER_ID		60										/*!< Block defined to store user id in RFID Cards used by CUBEE platform  */
#define RFID_APP_BLOCK_SIZE					16										/*!< Size of block to Mifare classic cards  */
#define RFID_APP_CRC_SIZE					2										/*!< Size of block to Mifare classic cards  */

/*! Struct to instantiate a Wifi Controller configuration */
typedef struct
{
	bool initialized;												/*!< Indicates if the Wifi Controller component was initialized */

} RfidApp_Cfg_t;

/**
 * @brief This function initializes a Wifi Controller configuration instance and all its dependencies.
 *
 * @param this - pointer to a Wifi Controller configuration instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK if the Wifi Controller configuration was successfully initialized.
 * @arg CUBEE_ERROR_CODE otherwise.
*/
CubeeErrorCode_t RfidApp_Cfg_init(RfidApp_Cfg_t * this);

/**
 * @brief This function gets a Wifi Controller configuration instance.
 *
	 * @return pointer to a Wifi Controller configuration instance
*/
RfidApp_Cfg_t* RfidApp_Cfg_getInstance(void);

/**
 * @brief This function checks if a Wifi Controller configuration instance is already initialized.
 *
 * @return
 * @arg true, if the Wifi Controller configuration instance is initialized.
 * @arg false, otherwise.
*/
bool RfidApp_Cfg_isInitialized(RfidApp_Cfg_t * this);

#endif /* RFID_APP_CFG_H_ */
 
