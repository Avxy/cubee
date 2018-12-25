/*
 * PANController.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "esp_log.h"

#include "PANController.h"

#define LOG_HEADER 				"[PAN CONTROLLER SVC] "


/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/


/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/
static PANController panController =
{
		.initialized = false,
		.mapPANControllerToBluetoothDataID = {CUBEE_RCV_CMD_VAL_IDX, CUBEE_SND_CMD_VAL_IDX, CUBEE_MONITOR_VAL_IDX},
};


/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/
CubeeErrorCode_t PANController_init(PANController* this)
{
	CubeeErrorCode_t cubeeError;

	/* Verify if the Notifier is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing service PAN Controller... \n");

	/* Init PAN controler configuration */
	this->panControllerCfg = PANController_Cfg_getInstance();
	cubeeError = PANController_Cfg_init(this->panControllerCfg);
    if (cubeeError != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "PAn controller config initialization failed\n");
        return CUBEE_ERROR_UNDEFINED;
    }

    /* Init external components */
    this->bluetoothService = Bluetooth_getInstance();
	cubeeError = Bluetooth_init(this->bluetoothService);
    if (cubeeError != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "Bluetooth initialization failed\n");
        return CUBEE_ERROR_UNDEFINED;
    }

	this->initialized = true;

	ESP_LOGI(LOG_HEADER,"Service PAN Controller initialized \n");

	return CUBEE_ERROR_OK;
}

PANController_Cfg * PANController_getConfig(PANController* this)
{
	return  this->panControllerCfg;
}

PANController * PANController_getInstance(void)
{
	return  &panController;
}

CubeeErrorCode_t PANController_receiveBluetoothCmd(PANController* this, uint8_t * data, uint16_t size)
{
	return Bluetooth_receiveCmd(this->bluetoothService, data ,size);
}

CubeeErrorCode_t PANController_receiveBluetoothConfig(PANController* this, uint8_t * data, uint16_t size)
{
	return Bluetooth_receiveCfg(this->bluetoothService, data ,size);
}

CubeeErrorCode_t PANController_receiveBluetoothDB9(PANController* this, uint8_t * data, uint16_t size)
{
	return Bluetooth_receiveDB9(this->bluetoothService, data ,size);
}

CubeeErrorCode_t PANController_sendBluetoothData(PANController* this, uint8_t * data, PANController_BluetoothDataId_t dataID, uint8_t size)
{
	return Bluetooth_setAtributeValue(this->bluetoothService, this->mapPANControllerToBluetoothDataID[dataID], data, (uint16_t)size);
}

CubeeErrorCode_t PANController_sendAuthenticationCommand(PANController* this)
{
	return Bluetooth_sendSignal(this->bluetoothService, BLUETOOTH_SIGNAL_AUTHENTICATED);
}

CubeeErrorCode_t PANController_enableBluetooth(PANController* this, bool status)
{
	if(status)
	{
		return Bluetooth_setAdvertingStatus(this->bluetoothService, BLUETOOTH_ADVERTISING_ENABLED);
	}

	return Bluetooth_setAdvertingStatus(this->bluetoothService, BLUETOOTH_ADVERTISING_DISABLED);
}

CubeeErrorCode_t PANController_setName(PANController* this, uint8_t * name)
{
	ESP_LOGI(LOG_HEADER, "Updating PAN configuration...");

	if(Bluetooth_setName(this->bluetoothService, name) != CUBEE_ERROR_OK)
	{
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER, "PAN configuration successfully updated");

	return CUBEE_ERROR_OK;
}


/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/


