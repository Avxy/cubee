/*
 * SensorMgr_Cfg.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef POWER_SENSOR_CFG_H_
#define POWER_SENSOR_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../util/StateMachine/FSM.h"

#define CURRENT_SENSOR_PERIOD					  	1000 / portTICK_PERIOD_MS
#define ACCELEROMETER_PERIOD					  	1000 / portTICK_PERIOD_MS
#define SENSOR_SEMAPHORE_TIMEOUT					3000 / portTICK_PERIOD_MS

#define SENSOR_BUFFER_SIZE	(600) /* Amostras = (Autonomia / Periodo de Amostragem) = (10min*60s*1000ms / 1000ms) */

typedef struct
{
	bool initialized;
} SensorMgr_Cfg_t;

/************************************************************************************************/
/* State Machine TypeDefs */
/************************************************************************************************/
typedef enum
{
	SENSOR_MGR__NOSTATE = 0, /* Explicit value specification added by code generator */
	SENSOR_MGR__INITIALIZATION,
	SENSOR_MGR__IDLE,
} SensorMgr_States_t;

typedef struct
{
    FSM_STATE mainState;
    FSM_STATE Idle;
    FSM_STATE Initialization;
} SensorMgr_STM_t;

typedef enum
{
    SENSOR_MGR_SIGNALS_NOSIG = 0, /* Explicit value specification added by code generator */
} SensorMgr_Signals_t;

SensorMgr_Cfg_t * SensorMgr_Cfg_getInstance();
CubeeErrorCode_t SensorMgr_Cfg_init(SensorMgr_Cfg_t* this);


#ifdef __cplusplus
}
#endif

#endif /* POWER_SENSOR_CFG_H_ */
