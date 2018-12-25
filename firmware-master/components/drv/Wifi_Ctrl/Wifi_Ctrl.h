/**
 * @file Wifi_Ctrl.h
 * @version 1.0
 * @author Alex Fernandes
 * @date June 06, 2017
 **************************************************************************
 *
 * @brief  Wifi Controller
 *
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   06-Jun-2017    Alex Fernandes
 * * Original version based on
 ******************************************************************/

#ifndef WIFI_CTRL_H_
#define WIFI_CTRL_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/*FreeRTOS libraries*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


/* Modules dependencies */
#include "Wifi_Ctrl_Cfg.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../util/StateMachine/FSM.h"

/************************************************************************************************/
/* State Machine TypeDefs */
/************************************************************************************************/

/*! Enum with possible states to the wifi controler state machine */
typedef enum
{
	WIFI_CTRL_NOSTATE = 0,				/*!< No state */
	WIFI_CTRL_STATE_INITIALIZATION,		/*!< Indicates that wifi is doing initialization task */
	WIFI_CTRL_STATE_CONFIGURATION,		/*!< Indicates that wifi is doing configuration task */
	WIFI_CTRL_STATE_CONNECTING,			/*!< Indicates that wifi is trying to connected with Access Point configured */
	WIFI_CTRL_STATE_CONNECTED,			/*!< Indicates that wifi is  connected with access point configured and got a valid IP*/
	WIFI_CTRL_STATE_SLEEPING,			/*!< Indicates that wifi is stopped */
} Wifi_Ctrl__States_t;

/*! Enum with signals used to stimulate the wifi controller state machine */
typedef enum
{
	WIFI_CTRL_NOSIG = 0,				/*!< No signal */
	WIFI_CTRL_SIGNAL_INITIALIZED,		/*!< Indicates that initializations task is finished */
	WIFI_CTRL_SIGNAL_CONFIGURED,		/*!< Indicates that configuration task is finished */
	WIFI_CTRL_SIGNAL_STARTED,			/*!< Indicates that wifi station was started */
	WIFI_CTRL_SIGNAL_STOPPED,			/*!< Indicates that wifi station was stopped */
	WIFI_CTRL_SIGNAL_CONNECTED,			/*!< Indicates that wifi station was connected with an wifi access point */
	WIFI_CTRL_SIGNAL_DISCONNECTED,		/*!< Indicates that wifi station was disconnected from an wifi access point */

	WIFI_CTRL_SIGNAL_MAX,				/*!< Indicates the number of possible signals */
} Wifi_Ctrl_Signals_t;

/*! Struct with state data to each possible state of the wifi controller state machine */
typedef struct
{
    FSM_STATE mainState;
    FSM_STATE Initialization;
    FSM_STATE Configuration;
    FSM_STATE Connecting;
    FSM_STATE Connected;
    FSM_STATE Sleeping;
} Wifi_Ctrl_STM_t;

/************************************************************************************************/
/* Wifi Controller TypeDefs */
/************************************************************************************************/


/*! Struct to instantiate a Wifi Controller component */
typedef struct
{
	bool initialized;										/*!< Indicates if the Wifi Controller component was initialized */
	Wifi_Ctrl_Cfg_t * wifiControllerCfg;					/*!< Pointer to the configurations used by the Wifi Controller instance */
	bool wifiEnable;										/*!< Flag used to enable or disable the wifi communication */
	SemaphoreHandle_t semaphore;							/*!< Semaphore used to synchronize access to wifi resources */
	QueueHandle_t signalQueueHandle;						/*!< Queue used to store state machine signals and stimulates the wifi controller state machine*/
	Wifi_Ctrl_STM_t stm;									/*!< Pointer to the wifi controller state machine data */
} Wifi_Ctrl_t;


/************************************************************************************************/
/* Public API */
/************************************************************************************************/

/**
 * @brief This function initializes a Wifi Controller component and all its dependencies.
 *
 * @param this - pointer to Wifi Controller component instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the component Wifi Controller initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t Wifi_Ctrl_init(Wifi_Ctrl_t * this);

/**
* @brief This function gets a Wifi Controller instance.
*
* @return pointer to a Wifi Controller component instance
*/
Wifi_Ctrl_t * Wifi_Ctrl_getInstance(void);

/**
* @brief This function gets a Wifi Controller configuration instance.
*
* @return pointer to the Wifi Controller configuration instance.
*/
Wifi_Ctrl_Cfg_t * Wifi_Ctrl_getCfg(Wifi_Ctrl_t* this);

/**
 * @brief This function checks if a Wifi Controller instance is already initialized.
 *
 * @return
 * @arg true, if the Wifi Controller instance is initialized.
 * @arg false, otherwise.
*/
bool Wifi_Ctrl_isInitialized(Wifi_Ctrl_t * this);

/**
 * @brief This function changes the SSID and Password used to connected the wifi station with the wifi access point
 *
 * @param this - pointer to Wifi Controller component instance.
 * @param ssid - pointer to access point SSID.
 * @param ssidSize - size of ssid string.
 * @param password - pointer to access point password.
 * @param passwordSize - size of access point password.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the SSID and Password were successfully updated
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t Wifi_Ctrl_setAccessPoint(Wifi_Ctrl_t * this, char * ssid, uint16_t ssidSize, char * password, uint16_t passwordSize );

/**
 * @brief This function verifies if the wifi is currently connected
 *
 * @param this - pointer to Wifi Controller component instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the wifi is currently connected
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t Wifi_Ctrl_isConnected(Wifi_Ctrl_t * this);

/**
 * @brief This function block until a wifi connection be established
 *
 * @param this - pointer to Wifi Controller component instance.
 * @param timeout - The max time, in milliseconds, to block waiting for a connection
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the connection was established within the timeout.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t Wifi_Ctrl_waitConnection(Wifi_Ctrl_t * this, uint32_t timeout);

/**
 * @brief
 *
 * @return
*/
CubeeErrorCode_t Wifi_Ctrl_updateCfg(Wifi_Ctrl_t * this);

/**
 * @brief This function sends a signal to the wifi controller driver
 *
 * @param this - pointer to wifi controller component instance.
 * @param sendsignal - signal to be sent
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the signal was successfully sent to the signal queue.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_TIMEOUT, if was not possible access the wifi shared resources
 * @arg CUBEE_ERROR_UNDEFINED, if it wasn't possible send the signal cause the signal queue is already full
*/
CubeeErrorCode_t Wifi_Ctrl_sendSignal(Wifi_Ctrl_t * this, Wifi_Ctrl_Signals_t rcvSignal);


#endif /* WIFI_CTRL_H_ */
