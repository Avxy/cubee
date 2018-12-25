/**
 * @file Wifi_Ctrl_Cfg.h
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

#ifndef WIFI_CTRL_CFG_H_
#define WIFI_CTRL_CFG_H_

#include "../../../main/ProjectCfg.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

//#define DEFAULT_WIFI_SSID 				"Lacina"
//#define DEFAULT_WIFI_PSWD 				"Lacina_21011901"

#define WIFI_CTRL_DEFAULT_WIFI_SSID 				"00000"
#define WIFI_CTRL_DEFAULT_WIFI_PSWD 				"00000"
#define WIFI_MAX_CONNECTION_ERROR		(5)
#define WIFI_MAX_RESTART_ATTEMPTS		(5)
#define WIFI_CTRL_CONNECTION_TIMEOUT_MS		(30000)									/*!< Timeout to get a valid wifi connection with an wifi  access point */
#define WIFI_CTRL_SLEEP_TIMEOUT_MS			(120000)								/*!< Timeout to wake-up and try reconnect the wifi station to an wifi access point */
#define WIFI_CTRL_SIGNAL_QUEUE_LENGTH		(5)										/*!< Length of queue used to store wifi controller state machine signals */
#define WIFI_CTRL_SIGNAL_QUEUE_ITEM_SIZE   	sizeof( Wifi_Ctrl_Signals_t )			/*!< Size of wifi controller state machine signals */
#define WIFI_CTRL_SIGNAL_QUEUE_TIMEOUT		(10 / portTICK_PERIOD_MS)				/*!< Waiting timeout to receive a signal from the wifi state machine queue */
#define WIFI_CTRL_SEMAPHORE_TIMEOUT			1000 / portTICK_PERIOD_MS				/*!< Waiting timeout to access bluetooth shared resources */

/*! Struct to instantiate a Wifi Controller configuration */
typedef struct
{
	uint8_t wifiSSID[SSID_MAX_SIZE];								/*!< Defines the Wifi SSID used to connect to the Wifi station */
	uint8_t wifiPassword[PASSWORD_MAX_SIZE];						/*!< Defines the Wifi password used to connect to the Wifi station */
	bool initialized;												/*!< Indicates if the Wifi Controller component was initialized */

} Wifi_Ctrl_Cfg_t;

/**
 * @brief This function initializes a Wifi Controller configuration instance and all its dependencies.
 *
 * @param this - pointer to a Wifi Controller configuration instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK if the Wifi Controller configuration was successfully initialized.
 * @arg CUBEE_ERROR_CODE otherwise.
*/
CubeeErrorCode_t Wifi_Ctrl_Cfg_init(Wifi_Ctrl_Cfg_t * this);

/**
 * @brief This function gets a Wifi Controller configuration instance.
 *
	 * @return pointer to a Wifi Controller configuration instance
*/
Wifi_Ctrl_Cfg_t* Wifi_Ctrl_Cfg_getInstance(void);

/**
 * @brief This function checks if a Wifi Controller configuration instance is already initialized.
 *
 * @return
 * @arg true, if the Wifi Controller configuration instance is initialized.
 * @arg false, otherwise.
*/
bool Wifi_Ctrl_Cfg_isInitialized(Wifi_Ctrl_Cfg_t * this);

#ifdef __cplusplus
}
#endif


#endif /* WIFI_CTRL_CFG_H_ */
 
