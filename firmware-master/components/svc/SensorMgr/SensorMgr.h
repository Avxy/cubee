/*
 * SensorMgr.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef SENSOR_MGR_H_
#define SENSOR_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../../svc/IOMgr/IOMgr.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../main/ProjectCfg.h"
#include "SensorMgr_Cfg.h"

/************************************************************************************************/
/* SensorMgr TypeDefs */
/************************************************************************************************/
typedef enum
{
#ifdef USING_CURRENT_SENSOR
	SENSOR_MGR_CURRENT_SENSOR_ID = 0,
#endif
#ifdef USING_ACCELEROMETER
	SENSOR_MGR_ACCELEROMETER_ID,
#endif

	SENSOR_MGR_ID_MAX,
} SensorMgr_SensorId_t;


typedef struct
{
	uint16_t samplePeriod;
	int16_t dataBuffer [SENSOR_BUFFER_SIZE];
	uint16_t sampleCounter;
	uint32_t lastSampleTime;
	CubeeErrorCode_t (*measuringFunction) (void *);
	SemaphoreHandle_t semaphore;
} SensorMgr_Sensor_t;



typedef struct
{
	bool initialized;
	SensorMgr_Sensor_t sensors[SENSOR_MGR_ID_MAX];
	SensorMgr_Cfg_t *sensorMgrCfg;
	IOMgr_t *ioMgr;
	SensorMgr_STM_t stm;
} SensorMgr_t;

/************************************************************************************************/
/* Public API */
/************************************************************************************************/
CubeeErrorCode_t SensorMgr_init(SensorMgr_t* this);
SensorMgr_Cfg_t * SensorMgr_getConfig(SensorMgr_t* this);
SensorMgr_t * SensorMgr_getInstance();
bool SensorMgr_isInitialized(SensorMgr_t* this);
bool SensorMgr_isEmpty(SensorMgr_t* this, SensorMgr_SensorId_t sensorId);
CubeeErrorCode_t SensorMgr_readAvrgValue(SensorMgr_t* this, SensorMgr_SensorId_t sensorId, int32_t* output_avrg);
CubeeErrorCode_t SensorMgr_readPeakValue(SensorMgr_t* this, SensorMgr_SensorId_t sensorId, int32_t* output_peakValue);
CubeeErrorCode_t SensorMgr_readSTDEV(SensorMgr_t* this, SensorMgr_SensorId_t sensorId, int32_t* output_stDeviation);
CubeeErrorCode_t SensorMgr_resetSensor(SensorMgr_t* this, SensorMgr_SensorId_t sensorId);
CubeeErrorCode_t SensorMgr_printSensorBuffer(SensorMgr_t* this, SensorMgr_SensorId_t sensorId);


#ifdef __cplusplus
}
#endif

#endif /* SENSOR_MGR_H_ */
