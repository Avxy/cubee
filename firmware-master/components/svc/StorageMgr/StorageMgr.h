/*
 * StorageMgr.h
 *
 *  Created on: 22 de jun de 2017
 *      Author: UFCG
 */

#ifndef _STORAGEMGR_H_
#define _STORAGEMGR_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "../../../main/ProjectCfg.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../drv/MemoryMgr/MemoryMgr.h"

#define	STORAGE_MGR_DATA_BUFFER_MAX_SIZE	MEMORY_MGR_DATA_BUFFER_MAX_SIZE				/*!< Specifies the maximum size allowed to read/write data in flash partition */


/*! Enum with CUBBE operation alarms  */
typedef enum
{
	CUBEE_ALARM_INITIALIZATION = 0x00,			/*!< Code to alarm generated on CUBEE initialization */
	CUBEE_ALARM_RFID = 0x01,					/*!< Code to alarm generated on RFID card events */
}CubeeApp_Alarm_Code_t;

/*!< Types definition to store sensor samples */
typedef struct
{
	uint32_t timeStamp;
	uint32_t value;
}StorageMgr_Sample_t;

typedef union
{
	StorageMgr_Sample_t sample;

	uint8_t sampleArray[sizeof(StorageMgr_Sample_t)];
} StorageMgr_SampleUnion_t;

/*!< Types definition to store application alarms */
typedef struct
{
	uint32_t timeStamp;
	uint32_t code;
	uint32_t value;
}StorageMgr_Alarm_t;

typedef union
{
	StorageMgr_Alarm_t alarm;

	uint8_t alarmArray[sizeof(StorageMgr_Alarm_t)];
} StorageMgr_AlarmUnion_t;

/*!< Types definition to store DB9 rules */
typedef struct
{
	uint32_t timeDuration;				/*!< Duration in seconds that the state shall be executed */
	uint8_t value;						/*!< Each bit of value indicate the state of a respective DB9 output */
}StorageMgr_DB9ruleState_t;

typedef struct
{
	bool valid;													/*!< Indicates if the rule is currently valid */
	uint8_t numberOfStates;										/*!< Number of rule states */
	StorageMgr_DB9ruleState_t ruleStates[DB9_RULE_STATE_MAX];	/*!< List of states composing the rule */
} StorageMgr_DB9rule_t;

/*!< Types definition to store CUBEE configuration */
typedef struct
{
	uint8_t idCubee[ID_CUBEE_MAX_SIZE];				/*!< Unique CUBEE identifier */
	uint8_t cubeeName[CUBEE_NAME_MAX_SIZE];			/*!< CUBEE name */
	uint8_t wifiSSID[SSID_MAX_SIZE];				/*!< Wifi SSID used to connected with an Wifi access point */
	uint8_t wifiPassword[PASSWORD_MAX_SIZE];		/*!< Wifi password used to connected with an Wifi access point */
}CubeeConfig_t;

/**
 * *****************************************************************************************************************************************
 * NEW IMPLEMENTATION
 * *****************************************************************************************************************************************
 */

/*! Struct to instantiate a Storage Manager component */
typedef struct
{
	bool initialized;															/*!< Indicates if the WAN Controller component was initialized */
	CubeeConfig_t currentCubeeCfg;												/*!< RAM copy of the current CUBEE configuration stored in flash memory */
	StorageMgr_DB9rule_t currentDB9Rule;										/*!< RAM copy of the current DB9 rule stored in flash memory */
	uint8_t * alarmDataBuffer;													/*!< Buffer used to read alarms from flash memory*/
	uint16_t alarmDataBufferSize;												/*!< Size of Buffer used to read alarms from flash memory*/
	uint8_t * sampleDataBuffer;													/*!< Buffer used to read samples from flash memory*/
	uint16_t sampleDataBufferSize;											/*!<Size of Buffer used to read samples from flash memory*/
	MemoryMgr_t * memoryMgr;													/*!< Memory manager instance used by the Storage Manager module */
} StorageMgr_t;
/**
 * *****************************************************************************************************************************************
 */

CubeeErrorCode_t StorageMgr_Init(StorageMgr_t * this);

/**
* @brief This function gets a StorageMgr instance.
*
* @return pointer to a StorageMgr component instance
*/
StorageMgr_t * StorageMgr_getInstance(void);


/**
 * @brief Read current CUBEE configuration from RAM memory
 *
 * @param this: Pointer to module instance
 * @param cubeeCfg: output parameter to copy the current CUBEE configuration
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_readCubeeCfg(StorageMgr_t * this, CubeeConfig_t* cubeeCfg);

/**
 * @brief Update the CUBEE configuration in both RAM and flash memory
 *
 * @param this: Pointer to module instance
 * @param cubeeCfg: new CUBEE configuration
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_updateCubeeCfg(StorageMgr_t * this, CubeeConfig_t* cubeeCfg);

/**
 * @brief Read current DB9 rule from RAM memory
 *
 * @param this: Pointer to module instance
 * @param db9Rule: output parameter to copy the current DB9 RUle
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_readDB9Rule(StorageMgr_t * this, StorageMgr_DB9rule_t * db9Rule);

/**
 * @brief Update the DB9 rule in both RAM and flash memory
 *
 * @param this: Pointer to module instance
 * @param db9Rule: new DB9 rule
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_updateDB9Rule(StorageMgr_t * this, StorageMgr_DB9rule_t * db9Rule);

/**
 * @brief Erase flash memory used to store samples
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_eraseSampleMemory(StorageMgr_t * this);

/**
 * @brief Save a sample in the flash memory
 *
 * @param this: Pointer to module instance
 * @param sampleUnion: sample to be saved in flash memory
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_saveSample(StorageMgr_t * this, StorageMgr_SampleUnion_t* sampleUnion);

/**
 * @brief Read samples stored in flash memory.
 *
 * The buffer and size to store samples read needs shall be passed on the module parameters
 * sampleBuffer and sampleBufferSize.
 *
 * @param this: Pointer to module instance
 * @param outputSampleCounter: output parameter to copy the number of samples read
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_readSamples(StorageMgr_t * this, uint16_t* outputSampleCounter);

/**
 * @brief Confirm the sample reading, so this samples can be removed from flash memory.
 * This operation shall be used after a call for StorageMgr_readSamples.
 *
 * @param this: Pointer to module instance
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_confirmReadSamples(StorageMgr_t * this);

/**
 * @brief Verifies if the sample memory is empty
 *
 * @param this: Pointer to module instance
 *
 * @return
 *        - true, the sample memory is empty;
 *        - false, otherwise.
 */
bool StorageMgr_isSampleMemoryEmpty(StorageMgr_t * this);

/**
 * @brief Erase flash memory used to store alarms
 *
 * @param this: Pointer to module instance
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_eraseAlarmMemory(StorageMgr_t * this);

/**
 * @brief Save an alarm in the flash memory
 *
 * @param this: Pointer to module instance
 * @param alarmUnion: alarm to be saved in flash memory
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_saveAlarm(StorageMgr_t * this, StorageMgr_AlarmUnion_t * alarmUnion);

/**
 * @brief Read alarms stored in flash memory.
 *
 * The buffer and size to store alarms read shall  be passed on the module parameters
 * alarmBuffer and alarmBufferSize.
 *
 * @param this: Pointer to module instance
 * @param outputAlarmCounter: output parameter to copy the number of alarms read
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_readAlarms(StorageMgr_t * this, uint16_t* outputAlarmCounter);

/**
 * @brief Confirm the alarm reading, so this alarms can be removed from flash memory.
 * This operation shall be used after a call for StorageMgr_readAlarms.
 *
 * @param this: Pointer to module instance
 *
 * @return
 *        - CUBEE_ERROR_OK, if the operation completed successfully;
 *        - CUBEE_ERROR_UNDEFINED, if something goes wrong.
 */
CubeeErrorCode_t StorageMgr_confirmReadAlarms(StorageMgr_t * this);

/**
 * @brief Verifies if the alarm memory is empty
 *
 * @param this: Pointer to module instance
 *
 * @return
 *        - true, the alarm memory is empty;
 *        - false, otherwise.
 */
bool StorageMgr_isAlarmMemoryEmpty(StorageMgr_t * this);


#endif /* _STORAGEMGR_H_ */
