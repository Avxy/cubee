/*
 * PANController.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef PANCONTROLLER_H_
#define PANCONTROLLER_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "PANController_Cfg.h"
#include "../../drv/Bluetooth/Bluetooth.h"

typedef enum BluetoothDataId
{
	PAN_CONTROLLER_RCV_CMD_ID = 0,
	PAN_CONTROLLER_SND_CMD_ID,
	PAN_CONTROLLER_DATA_MONITOR_ID,

	PAN_CONTROLLER_DATA_ID_MAX
} PANController_BluetoothDataId_t;

typedef struct PANController
{
	bool initialized;
	Bluetooth_AttributeBDIndex_t mapPANControllerToBluetoothDataID[PAN_CONTROLLER_DATA_ID_MAX];
	PANController_Cfg *panControllerCfg;
	Bluetooth_t * bluetoothService;
} PANController;


CubeeErrorCode_t PANController_init(PANController* this);

PANController_Cfg * PANController_getConfig(PANController* this);

PANController * PANController_getInstance(void);

CubeeErrorCode_t PANController_receiveBluetoothCmd(PANController* this, uint8_t * data, uint16_t size);

CubeeErrorCode_t PANController_receiveBluetoothConfig(PANController* this, uint8_t * data, uint16_t size);

CubeeErrorCode_t PANController_receiveBluetoothDB9(PANController* this, uint8_t * data, uint16_t size);

CubeeErrorCode_t PANController_sendBluetoothData(PANController* this, uint8_t * data, PANController_BluetoothDataId_t dataID, uint8_t size);

CubeeErrorCode_t PANController_sendAuthenticationCommand(PANController* this);

CubeeErrorCode_t PANController_enableBluetooth(PANController* this, bool status);

CubeeErrorCode_t PANController_setName(PANController* this, uint8_t * name);

#endif /* PANCONTROLLER_H_ */
