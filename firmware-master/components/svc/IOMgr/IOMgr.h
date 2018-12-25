/*
 * IOMgr.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef IO_MGR_H_
#define IO_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../drv/GPIO/GPIO.h"
#include "../../drv/ADC/ADC.h"
#include "../../drv/I2C_Master/I2C_Master.h"
#include "../../../main/ProjectCfg.h"
#include "IOMgr_Cfg.h"

#define ACCEL_AXIS_DATA_SIZE		(2)			/* Size of accelerometer data to each axis */
#define TEMP_DATA_SIZE				(1)			/* Size of temperature data */
#define GYRO_DATA_SIZE				(6)			/* Size of gyroscope data (X-axis,Y-axis,Z-axis) */

typedef struct
{
	bool initialized;
	IOMgr_Cfg_t *ioMgrCfg;
	GPIO_t *gpio;
	ADC_t *adc;
	I2C_Master_t *i2c;
} IOMgr_t;

CubeeErrorCode_t IOMgr_init(IOMgr_t* this);

IOMgr_Cfg_t * IOMgr_getConfig(IOMgr_t* this);

IOMgr_t * IOMgr_getInstance();

CubeeErrorCode_t IOMgr_activate(IOMgr_t* this);

CubeeErrorCode_t IOMgr_deactivate(IOMgr_t* this);

bool IOMgr_isActivated(IOMgr_t* this);

bool IOMgr_isInitialized(IOMgr_t* this);

bool IOMgr_hasButtonCmd(IOMgr_t* this);

CubeeErrorCode_t  IOMgr_toogleLED( IOMgr_t* this);

#ifdef USING_CURRENT_SENSOR
CubeeErrorCode_t  IOMgr_readCurrent( IOMgr_t* this, int16_t* outputCurrent);
#endif /* USING_CURRENT_SENSOR */

#ifdef USING_ACCELEROMETER
CubeeErrorCode_t  IOMgr_readAccelerometer( IOMgr_t* this, int16_t* outputXYZ);
#endif /* USING_ACCELEROMETER */

#ifdef USING_DB9
CubeeErrorCode_t  IOMgr_setDB9Output( IOMgr_t* this, uint8_t value);
#endif /* USING_DB9 */

#ifdef __cplusplus
}
#endif

#endif /* IO_MGR_H_ */
