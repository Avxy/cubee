/*
 * IOMgr_Cfg.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef IO_MGR_CFG_H_
#define IO_MGR_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#include "IOMgr_Cfg.h"

/*Accelerometer*/
#define PWR_MGMT_1 		(0x6B) 		/* Power management configuration register */
#define ACCEL_XOUT 		(0x3B) 		/* Initial accelerometer register (X-axis) */
#define ACCEL_YOUT 		(0x3D) 		/* Initial accelerometer register (X-axis) */
#define ACCEL_ZOUT 		(0x3F) 		/* Initial accelerometer register (X-axis) */
#define TEMP_OUT 		(0x41) 		/* Initial temperature register*/
#define GYRO_XOUT 		(0x43) 		/* Initial gyroscope register (X-axis) */

typedef struct IOMgr_Cfg
{
	bool initialized;

} IOMgr_Cfg_t;

IOMgr_Cfg_t * IOMgr_Cfg_getInstance();
CubeeErrorCode_t IOMgr_Cfg_init(IOMgr_Cfg_t* this);




#ifdef __cplusplus
}
#endif

#endif /* IO_MGR_CFG_H_ */
