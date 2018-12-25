/**
 * @file Bluetooth_CubeeEnergy_DB.h
 * @version 1.1
 * @author Alex Fernandes
 * @date August 01, 2017
 **************************************************************************
 *
 * @brief  Database implementation to CUBEE GATT server
 * This module implements the database used by the CUBEE application to do Bluetooth Low Energy (BLE) data exchange.
 * Through this database the CUBEE can send/receive commands, receive configuration and send status to connected devices.
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
 * * Code documentation
 ******************************************************************/

#ifndef BLUETOOTH_CUBEE_ENERGY_DB
#define BLUETOOTH_CUBEE_ENERGY_DB

/* Standard libraries */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* Modules dependencies */
#include "Bluetooth_CubeeEnergy_DB.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../../main/ProjectCfg.h"

/* Bluedroid includes */
#include "esp_gatt_defs.h"

#define CUBEE_SVC_UUID					(0x0030)	/*!< Custom service UUID defined to the CUBEE bluetooth service created */
#define CUBEE_RCV_CMD_VALUE_UUID		(0x0031)	/*!< Custom characteristic UUID defined to the receive commands from connected devices */
#define CUBEE_RCV_CMD_VALUE_SIZE		(1)			/*!< Size of commands received from connected devices */
#define CUBEE_SND_CMD_VALUE_UUID		(0x0032)	/*!< Custom characteristic UUID defined to send commands to connected devices */
#define CUBEE_SND_CMD_VALUE_SIZE		(1)			/*!< Size of commands sent to connected devices */
#define CUBEE_MONITOR_VALUE_UUID		(0x0041)	/*!< Custom characteristic UUID defined to send status information to connected devices */
#define CUBEE_MONITOR_VALUE_SIZE		(1)			/*!< Size of status data sent to connected devices */
#define CUBEE_CONFIG_VALUE_UUID			(0x0051)	/*!< Custom characteristic UUID defined to receive configuration from connected devices */
#define CUBEE_DB9_VALUE_SIZE			(1)			/*!< Size of DB9 command sent to connected devices */
#define CUBEE_DB9_VALUE_UUID			(0x0061)	/*!< Custom characteristic UUID defined to receive DB9 command data from connected devices */


/* GATT Server Attributes Table Index  */
typedef enum
{
	CUBEE_SVC_IDX = 0,			/*!< CUBEE service */
	CUBEE_RCV_CMD_CHAR_IDX,		/*!< Characteristic to receive commands*/
	CUBEE_RCV_CMD_VAL_IDX,
	CUBEE_SND_CMD_CHAR_IDX,		/*!< Characteristic to send commands*/
	CUBEE_SND_CMD_VAL_IDX,
	CUBEE_MONITOR_CHAR_IDX,		/*!< Characteristic to send status */
	CUBEE_MONITOR_VAL_IDX,
	CUBEE_CONFIG_CHAR_IDX,		/*!< Characteristic to receive configuration */
	CUBEE_CONFIG_VAL_IDX,
	CUBEE_DB9_CHAR_IDX,			/*!< Characteristic to receive db9 command */
	CUBEE_DB9_VAL_IDX,

	CUBEE_GATTS_ATTRIBUTE_MAX,	/*!< Indicates the number of attributes */
} Bluetooth_AttributeBDIndex_t;

/*! Struct to instantiate a CUBEE database */
typedef struct
{
	bool initialized;													/*!< Indicates if the i9nstance was initialized */
	esp_gatts_attr_db_t bluetoothGattsDB[CUBEE_GATTS_ATTRIBUTE_MAX];	/*!< BLE attribute table (database) */
} Bluetooth_CubeeEnergy_DB_t;

/**
 * @brief This function initializes the GATT server DB specific to CUBEE energy application.
 *
 * @param this - pointer to CUBEE Energy DB instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t Bluetooth_CubeeEnergyDB_init(Bluetooth_CubeeEnergy_DB_t* this);

/**
 * @brief This function gets a CUBEE Energy DB instance.
 *
 * @return pointer to a CUBEE Energy DB instance
*/
Bluetooth_CubeeEnergy_DB_t* Bluetooth_CubeeEnergyDB_getInstance();

/**
 * @brief This function checks if a CUBEE Energy DB instance is already initialized.
 *
 * @return
 * @arg true, if the CUBEE Energy DB instance is initialized.
 * @arg false, otherwise.
*/
bool Bluetooth_CubeeEnergyDB_isInitialized(Bluetooth_CubeeEnergy_DB_t* this);

#endif /* BLUETOOTH_CUBEE_ENERGY_DB */
 
