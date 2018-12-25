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

/* Modules Dependencies */
#include "Bluetooth_Cfg.h"

/* Bluedroid includes */
#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"

/* External libraries */
#include "esp_log.h"

#define LOG_HEADER 		"[BLUETOOTH_CFG] "	/*!< Header used to print LOG messages */

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

static Bluetooth_Cfg bluetoothCfgInstance =			/*!< Bluetooth configuration instance */
{
		.initialized = false,
		.bluetoothManufacturerData =
		{
		    /* LSB <--------------------------------------------------------------------------------> MSB */
			/* first uuid, 16bit, [12],[13] is the value */
		    0x00, 0x75, 0xCU, 0xBE, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0C, 0x0F, 0x10
		},
		.bluetoothServiceUUID128 =
		{
				/* LSB <--------------------------------------------------------------------------------> MSB */
				/* first uuid, 16bit, [12],[13] is the value */
				0x00 ,0x00 ,0x00 , 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEE, 0x00, 0xE0, 0xBE, 0xC0,
		},
		.bluetoothAdvParams =
		{
		    .adv_int_min        = 0x01FC, 				/* (160 ms) Time = N * 0.625 msec, Time Range: 20 ms to 10.24 sec */
		    .adv_int_max        = 0x01FC,				/* (1 segundo) Time = N * 0.625 msec, Time Range: 20 ms to 10.24 sec */
		    .adv_type           = ADV_TYPE_IND,
		    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
		    //.peer_addr            =
		    //.peer_addr_type       =
		    .channel_map        = ADV_CHNL_ALL,
		    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
		},
		.bluetoothAdvData =
		{
		    .set_scan_rsp = false,
		    .include_name = true,
		    .include_txpower = true,
		    .min_interval = 0x01FC, 	/*  Time = N * 0.625 msec, Time Range: 20 ms to 10.24 sec */
		    .max_interval = 0x01FC, 	/*  Time = N * 0.625 msec, Time Range: 20 ms to 10.24 sec */
		    .appearance = 0x00,
		    .manufacturer_len = sizeof(bluetoothCfgInstance.bluetoothManufacturerData),
		    .p_manufacturer_data =  bluetoothCfgInstance.bluetoothManufacturerData,
		    .service_data_len = 0,
		    .p_service_data = NULL,
		    .service_uuid_len = sizeof(bluetoothCfgInstance.bluetoothServiceUUID128),
		    .p_service_uuid = bluetoothCfgInstance.bluetoothServiceUUID128,
		    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT), /* 0x03 - Complete List of 16-bit Service Class UUIDs */
		},

};

/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t Bluetooth_Cfg_init(Bluetooth_Cfg * this)
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

	ESP_LOGI(LOG_HEADER,"Initializing Bluetooth configuration ...");

	this->bluetoothAttributeDB = Bluetooth_CubeeEnergyDB_getInstance();
	if(Bluetooth_CubeeEnergyDB_init(this->bluetoothAttributeDB) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"CUBEE database initialization failed ");
		return CUBEE_ERROR_UNDEFINED;
	}
	this->initialized = true;

	ESP_LOGI(LOG_HEADER," Bluetooth configuration successfully initialized");

	return CUBEE_ERROR_OK;
}

Bluetooth_Cfg* Bluetooth_Cfg_getInstance()
{
	return &bluetoothCfgInstance;
}

bool Bluetooth_Cfg_isInitialized(Bluetooth_Cfg * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

