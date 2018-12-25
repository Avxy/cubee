/**
 * @file WAN_Ctrl_Cfg.h
 * @version 2.0
 * @author Alex Fernandes
 * @date October 16, 2017
 **************************************************************************
 *
 * @brief  WAN Controller Configuration
 * This module has configurations used by the WAN Controller.
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   07-Jun-2017    Alex Fernandes
 * * Original version
 *
 *  Revision: 2.0   16-Oct-2017    Alex Fernandes
 * * Configuration refactoring for integration with MQTT service
 ******************************************************************/

#ifndef WAN_CTRL_CFG_H_
#define WAN_CTRL_CFG_H_

/* Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../../../main/ProjectCfg.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define WAN_CTRL_DATA_QUEUE_LENGTH			(1)									/*!< Length of queue used to store data received over WAN Network */
#define WAN_CTRL_DATA_QUEUE_ITEM_SIZE  		(sizeof(WAN_Ctrl_Data_Packet_t))	/*!< Size of data queue items */
#define WAN_CTRL_DATA_QUEUE_TIMEOUT			(1 / portTICK_PERIOD_MS)			/*!< Waiting timeout to receive data from queue */

/*! Struct to instantiate a WAN Controller configuration */
typedef struct
{
	uint8_t idCubee[ID_CUBEE_MAX_SIZE];								/*!< Defines the CUBEE ID used to communication with the server */
	uint8_t wifiSSID[SSID_MAX_SIZE];								/*!< Defines the Wifi SSID used to connect to the Wifi station */
	uint8_t wifiPassword[PASSWORD_MAX_SIZE];						/*!< Defines the Wifi password used to connect to the Wifi station */
	bool initialized;												/*!< Indicates if the WAN Controller component was initialized */

} WAN_Ctrl_Cfg_t;

/**
 * @brief This function initializes a WAN Controller configuration instance and all its dependencies.
 *
 * @param this - pointer to a WAN Controller configuration instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK if the WAN Controller configuration was successfully initialized.
 * @arg CUBEE_ERROR_CODE otherwise.
*/
CubeeErrorCode_t WAN_Ctrl_Cfg_init(WAN_Ctrl_Cfg_t * this);

/**
 * @brief This function gets a WAN Controller configuration instance.
 *
 * @return pointer to a WAN Controller configuration instance
*/
WAN_Ctrl_Cfg_t* WAN_Ctrl_Cfg_getInstance(void);

/**
 * @brief This function checks if a WAN Controller configuration instance is already initialized.
 *
 * @return
 * @arg true, if the WAN Controller configuration instance is initialized.
 * @arg false, otherwise.
*/
bool WAN_Ctrl_Cfg_isInitialized(WAN_Ctrl_Cfg_t * this);

#endif /* WAN_CTRL_CFG_H_ */
 
