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

/* Module Dependencies */
#include "Bluetooth_CubeeEnergy_DB.h"

/* External libraries */
#include "esp_log.h"

#define LOG_HEADER 		"[CUBEE_DATABASE] "	/*!< Header used to print LOG messages */

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

static uint16_t primaryServiceUUID = ESP_GATT_UUID_PRI_SERVICE;												/*!< Standard primary service UUID used to primary service declaration */
static uint16_t characterDeclarationUUID = ESP_GATT_UUID_CHAR_DECLARE;										/*!< Standard characteristic UUID used to characteristic declaration */
static const uint8_t characterPropRead = ESP_GATT_CHAR_PROP_BIT_READ;										/*!< Read permission */
static const uint8_t characterPropReadWrite = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_READ;		/*!< Read/write permission */
static uint16_t CubeeSvcUUID = CUBEE_SVC_UUID;																/*!< Custom service UUID defined to the CUBEE bluetooth service created */
static uint16_t RCVCmdValueUUID = CUBEE_RCV_CMD_VALUE_UUID;													/*!< Custom characteristic UUID defined to the receive commands from connected devices */
static uint8_t RCVCmdValue[CUBEE_RCV_CMD_VALUE_SIZE] = {0x00};												/*!< Buffer to store commands received from connected devices */
static uint16_t SNDCmdValueUUID = CUBEE_SND_CMD_VALUE_UUID;													/*!< Custom characteristic UUID defined to send commands to connected devices */
static uint8_t SNDCmdValue[CUBEE_SND_CMD_VALUE_SIZE] = {0x00};												/*!< Buffer to store commands sent to connected devices */
static uint16_t cubeeMonitorValueUUID = CUBEE_MONITOR_VALUE_UUID;											/*!< Custom characteristic UUID defined to send status information to connected devices */
static uint8_t cubeeMonitorValue[CUBEE_MONITOR_VALUE_SIZE] = {0x01};										/*!< Buffer to store status information sent to connected devices */
static uint16_t cubeeConfigValueUUID = CUBEE_CONFIG_VALUE_UUID;												/*!< Custom characteristic UUID defined to receive configuration from connected devices */
static uint8_t cubeeConfigValue[CUBEE_CONFIG_SERVICE_VALUE_SIZE] = {0x00};									/*!< Buffer to store configuration data received from connected devices */
static uint16_t cubeeDB9ValueUUID = CUBEE_DB9_VALUE_UUID;													/*!< Custom characteristic UUID defined to receive DB9 command from connected devices */
static uint8_t cubeeDB9Value[CUBEE_DB9_VALUE_SIZE] = {0x00};												/*!< Buffer to store configuration data received from connected devices */

static Bluetooth_CubeeEnergy_DB_t cubeeEnergyDB =
{
		.initialized = false,
		.bluetoothGattsDB =															/*!< Database declaration */
		{
		    [CUBEE_SVC_IDX] =														/*!< CUBEE Service Declaration */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{

							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&primaryServiceUUID,    			/*!< UUID value */
							.perm = ESP_GATT_PERM_READ,                  			/*!< Attribute permission */
							.max_length = sizeof(CubeeSvcUUID),       				/*!< Maximum length of the element*/
							.length = sizeof(CubeeSvcUUID),          				/*!< Current length of the element*/
							.value = (uint8_t *) &CubeeSvcUUID,    					/*!< Element value array*/
					},
		    },
		    [CUBEE_RCV_CMD_CHAR_IDX] =												/*!< CUBEE Receive Command Characteristic Declaration */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&characterDeclarationUUID,			/*!< UUID value */
							.perm = ESP_GATT_PERM_READ,                  			/*!< Attribute permission */
							.max_length = sizeof(characterPropReadWrite),   		/*!< Maximum length of the element*/
							.length = sizeof(characterPropReadWrite),       		/*!< Current length of the element*/
							.value = (uint8_t *) &characterPropReadWrite,   		/*!< Element value array*/
					}
		    },
		    [CUBEE_RCV_CMD_VAL_IDX] =												/*!< CUBEE Receive Command Characteristic Value */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&RCVCmdValueUUID,  				/*!< UUID value */
							.perm = ESP_GATT_PERM_READ |ESP_GATT_PERM_WRITE,  		/*!< Attribute permission */
							.max_length = sizeof(RCVCmdValue),      				/*!< Maximum length of the element*/
							.length = sizeof(RCVCmdValue),          				/*!< Current length of the element*/
							.value = (uint8_t *) &RCVCmdValue,      				/*!< Element value array*/
					}
		    },
		    [CUBEE_SND_CMD_CHAR_IDX] =												/*!< CUBEE Send Command Characteristic Declaration */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&characterDeclarationUUID,    		/*!< UUID value */
							.perm = ESP_GATT_PERM_READ,                  			/*!< Attribute permission */
							.max_length = sizeof(characterPropReadWrite),     		/*!< Maximum length of the element*/
							.length = sizeof(characterPropReadWrite),         		/*!< Current length of the element*/
							.value = (uint8_t *) &characterPropReadWrite,     		/*!< Element value array*/
					}
		    },
		    [CUBEE_SND_CMD_VAL_IDX] =												/*!< CUBEE Send Command Characteristic Value */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&SNDCmdValueUUID,  				/*!< UUID value */
							.perm = ESP_GATT_PERM_READ |ESP_GATT_PERM_WRITE,  		/*!< Attribute permission */
							.max_length = sizeof(SNDCmdValue),      				/*!< Maximum length of the element*/
							.length = sizeof(SNDCmdValue),          				/*!< Current length of the element*/
							.value = (uint8_t *) &SNDCmdValue,      				/*!< Element value array*/
					}
		    },
		    [CUBEE_MONITOR_CHAR_IDX] =												/*!< CUBEE Monitor Characteristic Declaration */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&characterDeclarationUUID,  		/*!< UUID value */
							.perm = ESP_GATT_PERM_READ,  							/*!< Attribute permission */
							.max_length = sizeof(characterPropRead),      			/*!< Maximum length of the element*/
							.length = sizeof(characterPropRead),          			/*!< Current length of the element*/
							.value = (uint8_t *) &characterPropRead,      			/*!< Element value array*/
					}
		    },
		    [CUBEE_MONITOR_VAL_IDX] =												/*!< CUBEE Monitor Characteristic Value */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&cubeeMonitorValueUUID, 			/*!< UUID value */
							.perm = ESP_GATT_PERM_READ,  							/*!< Attribute permission */
							.max_length = sizeof(cubeeMonitorValue),     			/*!< Maximum length of the element*/
							.length = sizeof(cubeeMonitorValue),         			/*!< Current length of the element*/
							.value = (uint8_t *) &cubeeMonitorValue,    			/*!< Element value array*/
					}
		    },
		    [CUBEE_CONFIG_CHAR_IDX] =												/*!< CUBEE Configuration Characteristic Declaration */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&characterDeclarationUUID, 		/*!< UUID value */
							.perm = ESP_GATT_PERM_READ,  							/*!< Attribute permission */
							.max_length = sizeof(characterPropReadWrite),      		/*!< Maximum length of the element*/
							.length = sizeof(characterPropReadWrite),          		/*!< Current length of the element*/
							.value = (uint8_t *) &characterPropReadWrite,      		/*!< Element value array*/
					}
		    },
		    [CUBEE_CONFIG_VAL_IDX] =
		    {
		    		.attr_control =													/*!< CUBEE Configuration Characteristic Value */
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&cubeeConfigValueUUID,  			/*!< UUID value */
							.perm = ESP_GATT_PERM_READ |ESP_GATT_PERM_WRITE,  		/*!< Attribute permission */
							.max_length = CUBEE_CONFIG_SERVICE_VALUE_SIZE,      	/*!< Maximum length of the element*/
							.length = sizeof(cubeeConfigValue),          			/*!< Current length of the element*/
							.value = (uint8_t *) &cubeeConfigValue,      			/*!< Element value array*/
					}
		    },
		    [CUBEE_DB9_CHAR_IDX] =													/*!< CUBEE DB9 Characteristic Declaration */
		    {
		    		.attr_control =
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&characterDeclarationUUID, 		/*!< UUID value */
							.perm = ESP_GATT_PERM_READ,  							/*!< Attribute permission */
							.max_length = sizeof(characterPropReadWrite),      		/*!< Maximum length of the element*/
							.length = sizeof(characterPropReadWrite),          		/*!< Current length of the element*/
							.value = (uint8_t *) &characterPropReadWrite,      		/*!< Element value array*/
					}
		    },
		    [CUBEE_DB9_VAL_IDX] =
		    {
		    		.attr_control =													/*!< CUBEE DB9 Characteristic Value */
		    		{
		    				.auto_rsp = ESP_GATT_AUTO_RSP,							/*!<attribute auto response flag */
		    		},
					.att_desc =
					{
							.uuid_length = ESP_UUID_LEN_16,				 			/*!< UUID length */
							.uuid_p = (uint8_t *)&cubeeDB9ValueUUID,			  	/*!< UUID value */
							.perm = ESP_GATT_PERM_WRITE,					  		/*!< Attribute permission */
							.max_length = CUBEE_DB9_VALUE_SIZE,      				/*!< Maximum length of the element*/
							.length = sizeof(cubeeDB9Value),          				/*!< Current length of the element*/
							.value = (uint8_t *) &cubeeDB9Value,      				/*!< Element value array*/
					}
		    },

		},
};

/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t Bluetooth_CubeeEnergyDB_init(Bluetooth_CubeeEnergy_DB_t* this)
{

	if(this == NULL)
	{
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(this->initialized != false)
	{
		/* Already initialized */
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing CUBEE bluetooth database ...");

	/**
	 * **** Insert initialization code here *******
	 */

	this->initialized = true;

	/**
	 * ********************************************
	 */

	ESP_LOGI(LOG_HEADER," CUBEE bluetooth database successfully initialized");


	return CUBEE_ERROR_OK;
}

Bluetooth_CubeeEnergy_DB_t* Bluetooth_CubeeEnergyDB_getInstance()
{
	return &cubeeEnergyDB;
}

bool Bluetooth_CubeeEnergyDB_isInitialized (Bluetooth_CubeeEnergy_DB_t* this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

