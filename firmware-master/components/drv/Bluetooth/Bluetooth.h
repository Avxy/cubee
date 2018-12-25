/**
 * @file Bluetooth.h
 * @version 1.1
 * @author Alex Fernandes
 * @date August 01, 2017
 **************************************************************************
 *
 * @brief  Driver Bluetooth
 * This module allows Bluetooth Low Energy (BLE) communication. It use the the Generic Access Profile (GAP) to broadcast advertising data
 * and connect to others BLE devices. Furthermore the driver has a GATT server that can be used to send/receive commands,
 * show the device status and receive configurations. The BLE driver implements a state machine to control the current state
 * and allows authentication on application level.
 *
 * @section References
 * @ref 1. BLUETOOTH SPECIFICATION Version 4.2
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   01-Jun-2017    Alex Fernandes
 * * Original version based on ESP-IDF security GATT server example
 *
 *  * Revision: 1.1   01-Aug-2017    Alex Fernandes
 * * Deep code refactoring and state machine implementation
 ******************************************************************/

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* FreeRTOS libraries */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

/* ESP-IDF Libraries */
#include "esp_bt_defs.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_defs.h"

/* Modules dependencies */
#include "Bluetooth_Cfg.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../util/StateMachine/FSM.h"


/************************************************************************************************/
/* State Machine TypeDefs */
/************************************************************************************************/

/*! Enum with possible states to the Bluetooth state machine */
typedef enum
{
	BLUETOOTH_NOSTATE = 0,				/*!< No state */
	BLUETOOTH_STATE_INITIALIZATION,		/*!< Indicates that bluetooth is doing initialization task */
	BLUETOOTH_STATE_CONFIGURATION,		/*!< Indicates that bluetooth is doing configuration task */
	BLUETOOTH_STATE_IDLE,				/*!< Indicates that bluetooth is ready to be used */
	BLUETOOTH_STATE_ADVERTISING,		/*!< Indicates that bluetooth is  advertising and can be found by other devices*/
	BLUETOOTH_STATE_AUTHENTICATING,		/*!< Indicates that bluetooth is connected with another device and waiting for authentication */
	BLUETOOTH_STATE_AUTHENTICATED,		/*!< Indicates that the bluetooth is connected with another device, authenticated and ready to exchange data */
} Bluetooth_States_t;

/*! Enum with signals used to stimulate the Bluetooth state machine */
typedef enum
{
	BLUETOOTH_NOSIG = 0,					/*!< No signal */
	BLUETOOTH_SIGNAL_INITIALIZED,			/*!< Indicates that initializations task is finished */
	BLUETOOTH_SIGNAL_CONFIGURED,			/*!< Indicates that configuration task is finished */
	BLUETOOTH_SIGNAL_ADVERTISNG_STARTED,	/*!< Indicates that bluetooth begins advertising */
	BLUETOOTH_SIGNAL_ADVERTISING_STOPPED,	/*!< Indicates that bluetooth stopped advertising */
	BLUETOOTH_SIGNAL_DEVICE_CONNECTED,		/*!< indicates that bluetooth is connected with another device */
	BLUETOOTH_SIGNAL_DEVICE_DISCONNECTED,	/*!< Indicates the bluetooth was disconnected from another device */
	BLUETOOTH_SIGNAL_AUTHENTICATED,			/*!< Indicates the authentication procedure was complete */

	BLUETOOTH_SIGNAL_MAX,					/*!< Indicates the number of possible signals */
} Bluetooth_Signals_t;

/*! Struct with state data to each possible state to the Bluetooth state machine */
typedef struct
{
    FSM_STATE mainState;
    FSM_STATE Configuration;
    FSM_STATE Idle;
    FSM_STATE Initialization;
    FSM_STATE Advertising;
    FSM_STATE Authenticating;
    FSM_STATE Authenticated;
} Bluetooth_STM_t;

/************************************************************************************************/
/* TypeDefs */
/************************************************************************************************/

/*! Enum with existing advertising status for the bluetooth module */
typedef enum
{
	BLUETOOTH_ADVERTISING_DISABLED = 0,		/*!< The bluetooth advertising is enabled */
	BLUETOOTH_ADVERTISING_ENABLED,			/*!< The bluetooth advertising is disabled */

	BLUETOOTH_ADVERTISING_STATUS_MAX,		/*!< Indicates the number of advertising status */
} Bluetooth_AdvertisingStatus_t;

/*
 * Gatt Server Profile Instance Type. The BLE device can run several GATT Server profile instances
 */
typedef struct
{
    esp_gatts_cb_t gatts_cb;
    esp_gatt_if_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
}Bluetooth_GattServerProfileInstance_t;

/*! Struct to instantiate a bluetooth component */
typedef struct
{
	bool initialized;										/*!< Indicates if the bluetooth component was initialized */
	Bluetooth_Cfg *bluetoothCfg;							/*!< Pointer to the configurations used by the Bluetooth instance */
	uint8_t bluetoothName[CUBEE_NAME_MAX_SIZE];				/*!< Bluetooth name */
	uint8_t peerAddress[ESP_BD_ADDR_LEN];					/*!< Bluetooth address for the device connected with bluetooth */
	Bluetooth_AdvertisingStatus_t advertisingStatus;		/*!< Defines if the bluetooth advertising is enabled or disabled */
	Bluetooth_GattServerProfileInstance_t gattsProfile;		/*!< Gatt Server Profile Instance */
	QueueHandle_t rcvCfgQueueHandle; 						/*!< Queue to store configuration received from connected devices */
	QueueHandle_t rcvcmdQueueHandle; 						/*!< Queue to store commands received from connected devices */
	QueueHandle_t rcvDB9QueueHandle; 						/*!< Queue to store DB9 commands received from connected devices */
	SemaphoreHandle_t semaphore;							/*!< Semaphore used to synchronize access to bluetooth resources */
	QueueHandle_t signalQueueHandle;						/*!< Queue used to store state machine signals and stimulates the bluetooth state machine*/
	Bluetooth_STM_t stm;									/*!< Pointer to the bluetooth state machine data */
} Bluetooth_t;


/************************************************************************************************/
/* Public API */
/************************************************************************************************/

/**
 * @brief This function initializes a bluetooth component and all its dependencies.
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the component bluetooth initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t Bluetooth_init(Bluetooth_t * this);

/**
* @brief This function gets a bluetooth instance.
*
* @return pointer to a bluetooth component instance
*/
Bluetooth_t * Bluetooth_getInstance(void);

/**
* @brief This function gets a Bluetooth configuration instance.
*
* @param this - pointer to bluetooth component instance.
*
* @return pointer to the Bluetooth configuration instance.
*/
Bluetooth_Cfg * Bluetooth_getCfg(Bluetooth_t* this);

/**
 * @brief This function checks if a bluetooth instance is already initialized.
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @return
 * @arg true, if the bluetooth instance is initialized.
 * @arg false, otherwise.
*/
bool Bluetooth_isInitialized(Bluetooth_t * this);

/**
 * @brief This function reads a command received by bluetooth.
 *
 * @param this - pointer to bluetooth component instance.
 * @param value - pointer to output command value.
 * @param this - size of output data.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the command reading was successful.
 * @arg CUBEE_ERROR_TIMEOUT, if it hasn't any command to be received
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
*/
CubeeErrorCode_t Bluetooth_receiveCmd(Bluetooth_t * this, uint8_t * value, uint16_t size);

/**
 * @brief This function read the device configuration data received by bluetooth.
 *
 * @param this - pointer to bluetooth component instance.
 * @param config - pointer to output configuration buffer.
 * @param size - size of output buffer.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the configuration was successfully received.
 * @arg CUBEE_ERROR_TIMEOUT, if it hasn't any configuration to be received
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
*/
CubeeErrorCode_t Bluetooth_receiveCfg(Bluetooth_t * this, uint8_t * config, uint16_t size);

/**
 * @brief This function read a DB9 command received by bluetooth.
 *
 * @param this - pointer to bluetooth component instance.
 * @param db9Cmd - pointer to output db9 buffer.
 * @param size - size of output buffer.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the db9 was successfully received.
 * @arg CUBEE_ERROR_TIMEOUT, if it hasn't any db9 command to be received
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
*/
CubeeErrorCode_t Bluetooth_receiveDB9(Bluetooth_t * this, uint8_t * db9Cmd, uint16_t size);

/**
 * @brief This function writes the attribute value in the GATT server.
 *
 * @param this - pointer to bluetooth component instance.
 * @param attribute - index of the attribute in the GATT server database.
 * @param value - value to be written.
 * @param this - size of value to be written.
 *s
 * @return
 * @arg CUBEE_ERROR_OK, if the attribute writing  was successful.
 * @arg CUBEE_ERROR_UNDEFINED, if it wasn't possible to write the attribute table
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
*/
CubeeErrorCode_t Bluetooth_setAtributeValue(Bluetooth_t * this, Bluetooth_AttributeBDIndex_t attribute, uint8_t * value, uint16_t size);

/**
 * @brief This function gets the current advertising status to the bluetooth driver.
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @return
 * @arg current advertising status
 * @arg BLUETOOTH_ADVERTISING_STATUS_MAX, if a invalid parameter was passed.
*/
Bluetooth_AdvertisingStatus_t Bluetooth_getAdvertingStatus(Bluetooth_t * this);

/**
 * @brief This function defines the advertising status to the bluetooth driver.
 *
 * @param this - pointer to bluetooth component instance.
 * @param status - new advertising status.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the advertising status was successfully configured.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
*/
CubeeErrorCode_t Bluetooth_setAdvertingStatus(Bluetooth_t * this, Bluetooth_AdvertisingStatus_t status);

/**
 * @brief This function sends a signal to the bluetooth driver
 *
 * @param this - pointer to bluetooth component instance.
 * @param sendsignal - signal to be sent
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the signal was successfully sent to the signal queue.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_TIMEOUT, if was not possible access the bluetooth shared resources
 * @arg CUBEE_ERROR_UNDEFINED, if it wasn't possible send the signal cause the signal queue is already full
*/
CubeeErrorCode_t Bluetooth_sendSignal(Bluetooth_t * this, Bluetooth_Signals_t sendsignal);

/**
 * @brief Defines the bluetooth name and updates the advertising data
 *
 * @param this - pointer to bluetooth component instance.
 * @param name - New bluetooth name.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the bluetooth name and advertising data were successfully updated.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_UNDEFINED, if the bluetooth name couldn't be updated
*/
CubeeErrorCode_t Bluetooth_setName(Bluetooth_t * this, uint8_t * name);

#endif /* BLUETOOTH_H_ */
