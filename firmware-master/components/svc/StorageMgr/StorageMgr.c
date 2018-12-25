/*
 * StorageMgr.c
 *
 *  Created on: 22 de jun de 2017
 *      Author: UFCG
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#include "StorageMgr.h"

#define LOG_HEADER 							"[STORAGE MGR SVC]"
#define DEBUG_STORAGE_MGR

// max length
#define MAX_SIZE 		20
#define CUBEE_NAME_DEFAULT		"CUBEE-new"
#define CUBEE_PASSWORD_DEFAULT	"Lacina_21011901"
#define CUBEE_SSID_DEFAULT		"Lacina"
#define CUBEE_ID_DEFAULT		"0"


/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

#ifdef DEBUG_STORAGE_MGR
static CubeeErrorCode_t __StorageMgr_printCubeeCfg(CubeeConfig_t * cubeeCfg);
static CubeeErrorCode_t __StorageMgr_printAlarm(StorageMgr_Alarm_t* alarm);
static CubeeErrorCode_t __StorageMgr_printSample(StorageMgr_Sample_t* sample);
#endif /* DEBUG_STORAGE_MGR */


/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/
static StorageMgr_t storageMgrInstance =
{

		.initialized = false,								/*!< Indicates if the WAN Controller component was initialized */
		.currentCubeeCfg =									/*!< RAM copy of the current CUBEE configuration stored in flash memory */
		{
				.idCubee = CUBEE_ID_DEFAULT,				/*!< Unique CUBEE identifier */
				.cubeeName = CUBEE_NAME_DEFAULT,			/*!< CUBEE name */
				.wifiSSID = CUBEE_SSID_DEFAULT,				/*!< Wifi SSID used to connected with an Wifi access point */
				.wifiPassword = CUBEE_PASSWORD_DEFAULT,		/*!< Wifi password used to connected with an Wifi access point */
		},
		.currentDB9Rule = 									/*!< RAM copy of the current DB9 rule stored in flash memory */
		{
				.valid = false,								/*!< Indicates if the rule is currently valid */
				.numberOfStates = 0,						/*!< Number of rule states */
		},
		.alarmDataBuffer = NULL,							/*!< Buffer used to read alarms from flash memory*/
		.alarmDataBufferSize = 0,							/*!< Size of Buffer used to read alarms from flash memory*/
		.sampleDataBuffer = NULL,							/*!< Buffer used to read samples from flash memory*/
		.sampleDataBufferSize = 0,							/*!<Size of Buffer used to read samples from flash memory*/
		.memoryMgr = NULL,									/*!< Memory manager instance used by the Storage Manager module */
};

static char defaultIdCubee[ID_CUBEE_MAX_SIZE] = CUBEE_ID_DEFAULT;				/*!< CUBEE identifier default*/
static char defaultCubeeName[CUBEE_NAME_MAX_SIZE] = CUBEE_NAME_DEFAULT;			/*!< CUBEE name default*/
static char defaultWifiSSID[SSID_MAX_SIZE] = CUBEE_SSID_DEFAULT;				/*!< Wifi SSID default */
static char defaultWifiPassword[PASSWORD_MAX_SIZE] = CUBEE_PASSWORD_DEFAULT;	/*!< Wifi password default*/

/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t StorageMgr_Init(StorageMgr_t * this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER initializing Storage Manager service");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(this->initialized != false)
	{
		/* Already initialized */
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing Storage Manager service ...");
	this->memoryMgr = MemoryMgr_getInstance();
	if(MemoryMgr_Init(this->memoryMgr) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Storage manager initialization failed, error initializing memory manager");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Read CUBEE configuration from flash memory */
	if(MemoryMgr_NVS_read(this->memoryMgr, MEMORY_MGR_NVS_DATA_TYPE_CFG, (uint8_t*) &this->currentCubeeCfg, sizeof(CubeeConfig_t)) != CUBEE_ERROR_OK)
	{
		memmove(this->currentCubeeCfg.idCubee, defaultIdCubee, ID_CUBEE_MAX_SIZE);
		memmove(this->currentCubeeCfg.cubeeName, defaultCubeeName, CUBEE_NAME_MAX_SIZE);
		memmove(this->currentCubeeCfg.wifiSSID, defaultWifiSSID, SSID_MAX_SIZE);
		memmove(this->currentCubeeCfg.wifiPassword, defaultWifiPassword, PASSWORD_MAX_SIZE);
	}
	/* Read DB9 rule from flash memory */
	if(MemoryMgr_NVS_read(this->memoryMgr, MEMORY_MGR_NVS_DATA_TYPE_DB9, (uint8_t*) &this->currentDB9Rule, sizeof(StorageMgr_DB9rule_t)) != CUBEE_ERROR_OK)
	{
		this->currentDB9Rule.valid = false;
		this->currentDB9Rule.numberOfStates = 0;
	}

	this->initialized = true;

    ESP_LOGI(LOG_HEADER,"Storage Manager successfully initialized");

	return CUBEE_ERROR_OK;
}

StorageMgr_t * StorageMgr_getInstance(void)
{
	return &storageMgrInstance;
}

CubeeErrorCode_t StorageMgr_readCubeeCfg(StorageMgr_t * this, CubeeConfig_t* cubeeCfg)
{
	if((this == NULL) || (cubeeCfg == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading CUBEE configuration in RAM memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Clean buffer */
	memset(cubeeCfg, 0, sizeof(CubeeConfig_t));

	/* Copy cubee configuration from RAM memory */
	memmove(cubeeCfg, &this->currentCubeeCfg, sizeof(CubeeConfig_t));

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t StorageMgr_updateCubeeCfg(StorageMgr_t * this, CubeeConfig_t* cubeeCfg)
{
	if((this == NULL) || (cubeeCfg == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER updating CUBEE configuration in RAM and flash memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Update configuration in flash memory */
	if(MemoryMgr_NVS_save(this->memoryMgr, MEMORY_MGR_NVS_DATA_TYPE_CFG, (uint8_t*) cubeeCfg, sizeof(CubeeConfig_t)) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating CUBEE configuration in RAM and flash memory");
		return CUBEE_ERROR_UNDEFINED;
	}
	/* Update configuration in RAM memory */
	memmove(&this->currentCubeeCfg, cubeeCfg, sizeof(CubeeConfig_t));

	ESP_LOGI(LOG_HEADER,"CUBEE configuration successfully updated in RAM and flash memory");
#ifdef DEBUG_STORAGE_MGR
	__StorageMgr_printCubeeCfg(cubeeCfg);
#endif

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t StorageMgr_readDB9Rule(StorageMgr_t * this, StorageMgr_DB9rule_t * db9Rule)
{
	if((this == NULL) || (db9Rule == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading DB9 rule in RAM memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Read DB9 rule */
	memset(db9Rule, 0, sizeof(StorageMgr_DB9rule_t));

	/* Copy DB9 rule from RAM memory */
	memmove(db9Rule, &this->currentDB9Rule, sizeof(StorageMgr_DB9rule_t));

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t StorageMgr_updateDB9Rule(StorageMgr_t * this, StorageMgr_DB9rule_t * db9Rule)
{
	if((this == NULL) || (db9Rule == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER updating DB9 rule in RAM and flash memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Update DB9 rule in flash memory */
	if(MemoryMgr_NVS_save(this->memoryMgr, MEMORY_MGR_NVS_DATA_TYPE_DB9, (uint8_t*) db9Rule, sizeof(StorageMgr_DB9rule_t)) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating DB9 rule in RAM and flash memory");
		return CUBEE_ERROR_UNDEFINED;
	}
	/* Update DB9 rule in RAM memory */
	memmove(&this->currentDB9Rule, db9Rule, sizeof(StorageMgr_DB9rule_t));

	ESP_LOGI(LOG_HEADER,"DB9 rule successfully updated in RAM and flash memory");
	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t StorageMgr_eraseSampleMemory(StorageMgr_t * this)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER erasing sample memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	return MemoryMgr_dataPartition_reset(this->memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE);
}

CubeeErrorCode_t StorageMgr_saveSample(StorageMgr_t * this, StorageMgr_SampleUnion_t* sampleUnion)
{
	if((this == NULL) || (sampleUnion == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER saving sample in flash memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Set the data and data size in memory manager */
	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBuffer = sampleUnion->sampleArray;
	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBufferSize = sizeof(StorageMgr_Sample_t);

	/* Push the sample to flash memory */
	if(MemoryMgr_dataPartition_push(this->memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error saving sample in flash memory");
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef DEBUG_STORAGE_MGR
	__StorageMgr_printSample(&sampleUnion->sample);
#endif /* DEBUG_STORAGE_MGR */

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t StorageMgr_readSamples(StorageMgr_t * this, uint16_t* outputSampleCounter)
{
#ifdef DEBUG_STORAGE_MGR
	StorageMgr_SampleUnion_t * samplePointer;
#endif /* DEBUG_STORAGE_MGR */

	if((this == NULL) || (this->sampleDataBuffer == NULL) || (this->sampleDataBufferSize == 0) || (outputSampleCounter == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading samples from flash memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBuffer = this->sampleDataBuffer;
	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBufferSize = (this->sampleDataBufferSize / sizeof(StorageMgr_Sample_t)) * sizeof(StorageMgr_Sample_t);
	if(MemoryMgr_dataPartition_read(this->memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error reading samples stored in flash memory");
		return CUBEE_ERROR_UNDEFINED;
	}
	*outputSampleCounter = this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBufferSize/sizeof(StorageMgr_Sample_t);

#ifdef DEBUG_STORAGE_MGR
		ESP_LOGI(LOG_HEADER,"%d Samples successfully read from memory: ", *outputSampleCounter);
		samplePointer = (StorageMgr_SampleUnion_t * ) this->sampleDataBuffer;
		for(int sampleIndex = 0; sampleIndex < (*outputSampleCounter); sampleIndex++)
		{
			__StorageMgr_printSample(&samplePointer[sampleIndex].sample);
		}
#endif /* DEBUG_STORAGE_MGR */

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t StorageMgr_confirmReadSamples(StorageMgr_t * this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER confirming sample reading");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBuffer = this->sampleDataBuffer;
	if(MemoryMgr_dataPartition_pop(this->memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error confirming sample reading");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

bool StorageMgr_isSampleMemoryEmpty(StorageMgr_t * this)
{
	return MemoryMgr_dataPartition_isEmpty(this->memoryMgr, MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE);
}

CubeeErrorCode_t StorageMgr_eraseAlarmMemory(StorageMgr_t * this)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER erasing alarm memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	return MemoryMgr_dataPartition_reset(this->memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_ALARM);
}

CubeeErrorCode_t StorageMgr_saveAlarm(StorageMgr_t * this, StorageMgr_AlarmUnion_t * alarmUnion)
{
	if((this == NULL) || (alarmUnion == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER saving alarm in flash memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Set the data and data size in memory manager */
	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_ALARM].dataBuffer = alarmUnion->alarmArray;
	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_ALARM].dataBufferSize = sizeof(StorageMgr_Alarm_t);

	/* Push the alarm to flash memory */
	if(MemoryMgr_dataPartition_push(this->memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_ALARM) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error saving alarm in flash memory");
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef DEBUG_STORAGE_MGR
	__StorageMgr_printAlarm(&alarmUnion->alarm);
#endif /* DEBUG_STORAGE_MGR */

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t StorageMgr_readAlarms(StorageMgr_t * this, uint16_t* outputAlarmCounter)
{
#ifdef DEBUG_STORAGE_MGR
	StorageMgr_AlarmUnion_t * alarmPointer;
#endif /* DEBUG_STORAGE_MGR */



	if((this == NULL) || (this->alarmDataBuffer == NULL) || (this->alarmDataBufferSize == 0) || (outputAlarmCounter == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading alarms from flash memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_ALARM].dataBuffer = this->alarmDataBuffer;
	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_ALARM].dataBufferSize = (this->alarmDataBufferSize / sizeof(StorageMgr_Alarm_t)) * sizeof(StorageMgr_Alarm_t);

	if(MemoryMgr_dataPartition_read(this->memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_ALARM) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error reading alarms stored in flash memory");
		return CUBEE_ERROR_UNDEFINED;
	}
	*outputAlarmCounter = this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_ALARM].dataBufferSize/sizeof(StorageMgr_Alarm_t);

#ifdef DEBUG_STORAGE_MGR
		ESP_LOGI(LOG_HEADER,"%d Alarms successfully read from memory: ", *outputAlarmCounter);
		alarmPointer = (StorageMgr_AlarmUnion_t * ) this->alarmDataBuffer;
		for(int alarmIndex = 0; alarmIndex < (*outputAlarmCounter); alarmIndex++)
		{
			__StorageMgr_printAlarm(&alarmPointer[alarmIndex].alarm);
		}
#endif /* DEBUG_STORAGE_MGR */

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t StorageMgr_confirmReadAlarms(StorageMgr_t * this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER confirming alarm reading");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	this->memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_ALARM].dataBuffer = this->alarmDataBuffer;
	if(MemoryMgr_dataPartition_pop(this->memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_ALARM) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error confirming alarm reading");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

bool StorageMgr_isAlarmMemoryEmpty(StorageMgr_t * this)
{
	return MemoryMgr_dataPartition_isEmpty(this->memoryMgr, MEMORY_MGR_PARTITION_DATA_TYPE_ALARM);
}


/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

#ifdef DEBUG_STORAGE_MGR
static CubeeErrorCode_t __StorageMgr_printCubeeCfg(CubeeConfig_t * cubeeCfg)
{
	if(cubeeCfg == NULL)
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER printing CUBEE configuration");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	ESP_LOGI(LOG_HEADER,"-------Cubee configuration-----");
	ESP_LOGI(LOG_HEADER,"CUBEE Name: %s", (char *)cubeeCfg->cubeeName);
	ESP_LOGI(LOG_HEADER,"CUBEE Id: %s", (char *)cubeeCfg->idCubee);
	ESP_LOGI(LOG_HEADER,"Wifi Password: %s", (char *)cubeeCfg->wifiPassword);
	ESP_LOGI(LOG_HEADER,"Wifi SSID: %s", (char *)cubeeCfg->wifiSSID);
	ESP_LOGI(LOG_HEADER,"-------------------------------");

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __StorageMgr_printAlarm(StorageMgr_Alarm_t* alarm)
{
	if(alarm == NULL)
	{
		ESP_LOGE("CUBEE_ERROR_INVALID_PARAMETER printing alarm",);
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	   ESP_LOGI(LOG_HEADER,"Alarm - timestamp: %d, code: %d, value: %d",
	    	alarm->timeStamp, alarm->code, alarm->value);

    return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __StorageMgr_printSample(StorageMgr_Sample_t* sample)
{
	if(sample == NULL)
	{
		ESP_LOGE("CUBEE_ERROR_INVALID_PARAMETER printing sample",);
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

    ESP_LOGI(LOG_HEADER,"Sample - timestamp: %d, value: %d",
    	sample->timeStamp, sample->value);

    return CUBEE_ERROR_OK;
}
#endif /* DEBUG_STORAGE_MGR */


