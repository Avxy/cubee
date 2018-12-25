/**
 * @file Bluetooth_Cfg.h
 * @version 1.1
 * @author Alex Fernandes
 * @date August 01, 2017
 **************************************************************************
 *
 * @brief  Bluetooth Configuration
 * This module has configurations used by the bluetooth driver.
 * It defines, among other things, the bluetooth advertising settings.
 *
 * @section References
 * @ref 1. BLUETOOTH SPECIFICATION Version 4.2
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   01-Jun-2017    Alex Fernandes
 * * Original version
 *
 * Revision: 1.1   01-Aug-2017    Alex Fernandes
 * * Code refactoring and documentation
 ******************************************************************/

#ifndef BLUETOOTH_CFG_H_
#define BLUETOOTH_CFG_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* Bluedroid includes */
#include "esp_gap_ble_api.h"
#include "../../../main/ProjectCfg.h"

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "Bluetooth_CubeeEnergy_DB.h"

#define BLUETOOTH_APP_ID						(0xAA)									/*!< GATT server profile ID */
#define CUBEE_BLUETOOTH_SVC_INSTANCE_ID			(0)										/*!< GATT server service ID*/
#define MANUFACTURER_DATA_SIZE					(20)									/*!< Manufacturer data size used into advertising data */
#define BLUETOOTH_AUTHENTICATION_TIMEOUT_MS		(15000)									/*!< Timeout to authentication procedure after connected with a remote device */
#define BLUETOOTH_SEMAPHORE_TIMEOUT				1000 / portTICK_PERIOD_MS				/*!< Waiting timeout to access bluetooth shared resources */
#define DEFAULT_ADVERTISING_STATUS 				(BLUETOOTH_ADVERTISING_ENABLED)			/*!< Defines the default advertising status */
#define RCV_CMD_QUEUE_LENGTH    				(1)										/*!< Length of queue used to store commands received from connected devices */
#define RCV_CMD_QUEUE_ITEM_SIZE       			sizeof( uint8_t )						/*!< Size of commands received from connected devices */
#define RCV_CMD_QUEUE_TIMEOUT					(100 / portTICK_PERIOD_MS)				/*!< Waiting timeout to receive a command from queue */
#define BLUETOOTH_SIGNAL_QUEUE_LENGTH			(10)									/*!< Length of queue used to store bluetooth state machine signals */
#define BLUETOOTH_SIGNAL_QUEUE_ITEM_SIZE   		sizeof( Bluetooth_Signals_t )			/*!< Size of bluetooth state machine signals */
#define BLUETOOTH_SIGNAL_QUEUE_TIMEOUT			(100 / portTICK_PERIOD_MS)				/*!< Waiting timeout to receive a signal from the bluetooth state machine queue */
#define CFG_QUEUE_LENGTH    					(1)										/*!< Length of queue used to store configuration received from connected devices */
#define CFG_QUEUE_ITEM_SIZE       				CUBEE_CONFIG_SERVICE_VALUE_SIZE			/*!< Size of configuration received from connected devices */
#define CFG_QUEUE_TIMEOUT						(100 / portTICK_PERIOD_MS)				/*!< Waiting timeout to receive a configuration from the queue */
#define DB9_QUEUE_LENGTH    					(1)										/*!< Length of queue used to store configuration received from connected devices */
#define DB9_QUEUE_ITEM_SIZE       				CUBEE_DB9_VALUE_SIZE						/*!< Size of db9 command received from connected devices */
#define DB9_QUEUE_TIMEOUT						(100 / portTICK_PERIOD_MS)				/*!< Waiting timeout to receive a db9 command from the queue */

/*! Struct to instantiate a bluetooth configuration */
typedef struct Bluetooth_Cfg
{
	bool initialized;												/*!< Indicates if the bluetooth component was initialized */
	Bluetooth_CubeeEnergy_DB_t *bluetoothAttributeDB;					/*!< Pointer to the GAT server database used by the Bluetooth instance */
	esp_ble_adv_params_t bluetoothAdvParams;						/*!< Bluetooth advertising parameters */
	esp_ble_adv_data_t bluetoothAdvData;							/*!< Bluetooth advertising data */
	uint8_t bluetoothManufacturerData[MANUFACTURER_DATA_SIZE];		/*!< Specific manufacturer data used into advertising data */
	uint8_t bluetoothServiceUUID128[ESP_UUID_LEN_128];				/*!< Bluetooth service UUID */
} Bluetooth_Cfg;

/**
 * @brief This function initializes a bluetooth configuration instance and all its dependencies.
 *
 * @param this - pointer to a bluetooth configuration instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the Bluetooth configuration was successfully initialized.
 * @arg CUBEE_ERROR_UNDEFINED ,if the initialization failed.
*/
CubeeErrorCode_t Bluetooth_Cfg_init(Bluetooth_Cfg * this);

/**
 * @brief This function gets a bluetooth configuration instance.
 *
 * @return pointer to a bluetooth configuration instance
*/
Bluetooth_Cfg* Bluetooth_Cfg_getInstance(void);

/**
 * @brief This function checks if a bluetooth configuration instance is already initialized.
 *
 * @return
 * @arg true, if the bluetooth configuration instance is initialized.
 * @arg false, if the bluetooth configuration instance isn't initialized or an invalid parameter was passed.
*/
bool Bluetooth_Cfg_isInitialized(Bluetooth_Cfg * this);

#endif /* BLUETOOTH_CFG_H_ */
 
