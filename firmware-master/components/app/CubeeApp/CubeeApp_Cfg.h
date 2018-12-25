/**
 * @file CubeeApp_Cfg.h
 * @version 1.0
 * @author Alex Fernandes
 * @date June 01, 2017
 **************************************************************************
 *
 * @brief  CubeeApp Configuration
 *
 ** This module has configurations used by the CubeeApp service.
 *
 * @section References
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   01-Jun-2017    Alex Fernandes
 * * Original version
 ******************************************************************/

#ifndef CUBEE_APP_CFG_H_
#define CUBEE_APP_CFG_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* Module dependecies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define WAN_SEMAPHORE_TIMEOUT					(5000 / portTICK_PERIOD_MS)				/*!< Timeout to take WAN network semaphore defined in CUBEE APP */
#define PAN_SEMAPHORE_TIMEOUT					(3000 / portTICK_PERIOD_MS)				/*!< Timeout to take PAN network semaphore defined in CUBEE APP */
#define PAN_BUFFER_SIZE							CUBEE_CONFIG_SERVICE_VALUE_SIZE			/*!< Size of buffer used to receive data from PAN network */
#define SEND_NOTIFICATION_PERIOD				(60000 / portTICK_PERIOD_MS)			/*!< Period to send notifications (alarm and measurement) over WAn network */
#define CUBEE_REG_ACK_TIMEOUT					(20)									/*!< Time in seconds to wait for CUBEE registration server acknowledgment*/
#define INSTANT_ALARM_QUEUE_LENGTH				(5)										/*!< Length of queue used to store instant alarms */
#define	INSTANT_ALARM_QUEUE_ITEM_SIZE			sizeof(StorageMgr_Alarm_t)				/*!< Size of items stored on instant alarm queue */
#define	INSTANT_ALARM_QUEUE_TIMEOUT				(3000 / portTICK_PERIOD_MS)				/*!< Timeout used in operation with the instant alarm queue */

/*! Struct to instantiate a CubeeApp configuration */
typedef struct
{
	bool initialized;
} CubeeApp_Cfg_t ;

/**
* @brief This function gets a CubeeApp configuration instance.
*
* @return pointer to the CubeeApp configuration instance
*/
CubeeApp_Cfg_t * CubeeApp_Cfg_getInstance(void);

/**
 * @brief This function initializes a CubeeApp configuration instance and all its dependencies.
 *
 * @param this - pointer to a CubeeApp configuration instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK if the CubeeApp configuration was successfully initialized.
 * @arg CUBEE_ERROR_CODE otherwise.
*/
CubeeErrorCode_t CubeeApp_Cfg_init(CubeeApp_Cfg_t * this);

/**
 * @brief This function checks if a CubeeApp configuration instance is already initialized.
 *
 * @return
 * @arg true, if the CubeeApp configuration instance is initialized.
 * @arg false, otherwise.
*/
bool CubeeApp_Cfg_isInitialized(CubeeApp_Cfg_t * this);



#endif /* CUBEE_APP_CFG_H_ */
