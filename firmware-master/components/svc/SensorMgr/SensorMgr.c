/*
 * SensorMgr.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "SensorMgr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "math.h"


#define DEBUG_SENSOR_MGR
#define LOG_HEADER		"[SENSOR_MGR SVC] "

#ifdef USING_SENSOR_MGR

/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/

/* SensorMgr task code */
void __SensorMgr_taskCode( void * pvParameters );

/* Handles the state machine */
bool __SensorMgr_STMStimulate(SensorMgr_t* this, SensorMgr_STM_t* stm, SensorMgr_Signals_t msg);

/* Initialize the state machine */
void __SensorMgr_STM_init(SensorMgr_t* this, SensorMgr_STM_t * stm);



/************************************************************************************************/
/* State Machine Conditions  Definition */
/************************************************************************************************/

bool __SensorMgr_isMeasuringTime(SensorMgr_t* this, SensorMgr_SensorId_t sensorId);


/************************************************************************************************/
/* State Machine Actions  Definition */
/************************************************************************************************/
#ifdef USING_CURRENT_SENSOR
CubeeErrorCode_t __SensorMgr_measureCurrentSensor(void * this);
#endif

#ifdef USING_ACCELEROMETER
CubeeErrorCode_t __SensorMgr_measureAccelerometer(void * this);
#endif

/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/

static SensorMgr_t sensorMgrInstance =
{
		.initialized = false,
		.sensorMgrCfg = NULL,
		.ioMgr = NULL,

		.sensors =
		{
#ifdef USING_CURRENT_SENSOR
			[SENSOR_MGR_CURRENT_SENSOR_ID] =
			{
				.samplePeriod = CURRENT_SENSOR_PERIOD,
				.dataBuffer = {0},
				.sampleCounter = 0,
				.lastSampleTime = 0,
				.semaphore = NULL,
				.measuringFunction = __SensorMgr_measureCurrentSensor,
			},
#endif
#ifdef USING_ACCELEROMETER
			[SENSOR_MGR_ACCELEROMETER_ID] =
			{
				.samplePeriod = ACCELEROMETER_PERIOD,
				.dataBuffer = {0},
				.sampleCounter = 0,
				.lastSampleTime = 0,
				.semaphore = NULL,
				.measuringFunction = __SensorMgr_measureAccelerometer,
			},
#endif
		},
};

/************************************************************************************************/
/* Public API  Implementation */
/************************************************************************************************/
/* Operation init of class Notifier */
CubeeErrorCode_t SensorMgr_init(SensorMgr_t* this)
{
	CubeeErrorCode_t error = CUBEE_ERROR_OK;
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	/* Verify if the SensorMgr is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing service SensorMgr...");

	/* Init SensorMgr config */
	this->sensorMgrCfg = SensorMgr_Cfg_getInstance();
	SensorMgr_Cfg_init(this->sensorMgrCfg);

	/* Init external components */
	this->ioMgr = IOMgr_getInstance();
	error = IOMgr_init(this->ioMgr);
	if(error != CUBEE_ERROR_OK){
		ESP_LOGE(LOG_HEADER, "IOMgr initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

    /* Create Mutex to access sensor resources */
	for(uint8_t sensorId = 0; sensorId < SENSOR_MGR_ID_MAX; sensorId ++)
	{
		this->sensors[sensorId].semaphore = xSemaphoreCreateMutex();
	    if(this->sensors[sensorId].semaphore == NULL)
	    {
	    	ESP_LOGE(LOG_HEADER, "Error creating semaphore to sensor %d", sensorId);
	    	return CUBEE_ERROR_UNDEFINED;
	    }
	}

    /* Call init function of stm  for instance  */
    __SensorMgr_STM_init(this, &(this->stm));

    /* Create the task, storing the handle. */
    xReturned = xTaskCreatePinnedToCore(
    					__SensorMgr_taskCode,      	/* Function that implements the task. */
						"SensorMgr_task", 			/* Text name for the task. */
						4096,      					/* Stack size in words, not bytes. */
						( void * ) this,    		/* Parameter passed into the task. */
						SENSOR_MGR_TASK_PRIORITY,	/* Priority at which the task is created. */
						&xHandle,					/* Used to pass out the created task's handle. */
						SENSOR_MGR_TASK_CORE);     	/* Specifies the core to run the task */
	if( xReturned != pdPASS )
    {
        return CUBEE_ERROR_UNDEFINED;
    }

	this->initialized = true;

	ESP_LOGI(LOG_HEADER,"Service SensorMgr initialized");

	return CUBEE_ERROR_OK;
}

SensorMgr_t * SensorMgr_getInstance()
{
	return &sensorMgrInstance;
}

SensorMgr_Cfg_t * SensorMgr_getConfig(SensorMgr_t* this)
{
	return this->sensorMgrCfg;
}

bool SensorMgr_isInitialized(SensorMgr_t* this)
{
	return this->initialized;
}

bool SensorMgr_isEmpty(SensorMgr_t* this, SensorMgr_SensorId_t sensorId)
{
	bool isEmpty;

	if((this == NULL) || (sensorId > SENSOR_MGR_ID_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER accessing sensor buffer");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->sensors[sensorId].semaphore, SENSOR_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking sensor semaphore, sensor ID; %d", sensorId);
		return CUBEE_ERROR_TIMEOUT;
	}

	/* Clean the sensor buffer and sample counter*/
	isEmpty = (this->sensors[sensorId].sampleCounter == 0);

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->sensors[sensorId].semaphore);

	return  isEmpty;
}

CubeeErrorCode_t SensorMgr_readAvrgValue(SensorMgr_t* this, SensorMgr_SensorId_t sensorId, int32_t* output_avrg)
{
	int32_t sum = 0;

	if((this == NULL) || (sensorId > SENSOR_MGR_ID_MAX) || (output_avrg == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading sensor average value");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->sensors[sensorId].semaphore, SENSOR_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking sensor semaphore, sensor ID; %d", sensorId);
		return CUBEE_ERROR_TIMEOUT;
	}

	if(this->sensors[sensorId].sampleCounter == 0)
	{
		ESP_LOGE(LOG_HEADER,"Error  reading sensor average valuet, the sample buffer is empty");
		/* We have finished accessing the shared resource.  Release the	 semaphore. */
		xSemaphoreGive( this->sensors[sensorId].semaphore);
		return CUBEE_ERROR_UNDEFINED;
	}

	/* sum of stored samples */
	for(uint16_t sampleIndex = 0; sampleIndex < this->sensors[sensorId].sampleCounter; sampleIndex++)
	{
		sum += this->sensors[sensorId].dataBuffer[sampleIndex];
	}
	/* Calculates the average*/
	(*output_avrg)  = sum / this->sensors[sensorId].sampleCounter;

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->sensors[sensorId].semaphore);

#ifdef DEBUG_SENSOR_MGR
	ESP_LOGI(LOG_HEADER,"Average value calculated: %d, sensor ID: %d, samples: %d", (*output_avrg), sensorId,  this->sensors[sensorId].sampleCounter);
#endif

	return  CUBEE_ERROR_OK;

}

CubeeErrorCode_t SensorMgr_readPeakValue(SensorMgr_t* this, SensorMgr_SensorId_t sensorId, int32_t* output_peakValue)
{
	if((this == NULL) || (sensorId > SENSOR_MGR_ID_MAX) || (output_peakValue == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading sensor peak value");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->sensors[sensorId].semaphore, SENSOR_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking sensor semaphore, sensor ID; %d", sensorId);
		return CUBEE_ERROR_TIMEOUT;
	}

	if(this->sensors[sensorId].sampleCounter == 0)
	{
		ESP_LOGE(LOG_HEADER,"Error  reading sensor peak value, the sample buffer is empty");
		/* We have finished accessing the shared resource.  Release the	 semaphore. */
		xSemaphoreGive( this->sensors[sensorId].semaphore);
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Initialize the peak value*/
	(*output_peakValue) = 0;

	/* sum of stored samples */
	for(uint16_t sampleIndex = 0; sampleIndex < this->sensors[sensorId].sampleCounter; sampleIndex++)
	{
		if(abs(this->sensors[sensorId].dataBuffer[sampleIndex]) > (*output_peakValue))
		{
			(*output_peakValue) = abs(this->sensors[sensorId].dataBuffer[sampleIndex]);
		}
	}

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->sensors[sensorId].semaphore);

#ifdef DEBUG_SENSOR_MGR
	ESP_LOGI(LOG_HEADER,"Peak value calculated: %d, sensor ID: %d, samples: %d", (*output_peakValue), sensorId,  this->sensors[sensorId].sampleCounter);
#endif

	return  CUBEE_ERROR_OK;
}

CubeeErrorCode_t SensorMgr_readSTDEV(SensorMgr_t* this, SensorMgr_SensorId_t sensorId, int32_t* output_stDeviation)
{
	int32_t sampleAverage = 0;
	uint64_t powSum = 0;

	if((this == NULL) || (sensorId > SENSOR_MGR_ID_MAX) || (output_stDeviation == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading sensor standard deviation value");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->sensors[sensorId].semaphore, SENSOR_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking sensor semaphore, sensor ID; %d", sensorId);
		return CUBEE_ERROR_TIMEOUT;
	}

	if(this->sensors[sensorId].sampleCounter == 0)
	{
		ESP_LOGE(LOG_HEADER,"Error  reading sensor reading sensor standard deviation value, the sample buffer is empty");
		/* We have finished accessing the shared resource.  Release the	 semaphore. */
		xSemaphoreGive( this->sensors[sensorId].semaphore);
		return CUBEE_ERROR_UNDEFINED;
	}

	/* sum of stored samples */
	for(uint16_t sampleIndex = 0; sampleIndex < this->sensors[sensorId].sampleCounter; sampleIndex++)
	{
		sampleAverage += this->sensors[sensorId].dataBuffer[sampleIndex];
	}

	sampleAverage /= this->sensors[sensorId].sampleCounter;

	for(uint16_t sampleIndex = 0; sampleIndex < this->sensors[sensorId].sampleCounter; sampleIndex++)
	{
		powSum += pow(this->sensors[sensorId].dataBuffer[sampleIndex] - sampleAverage, 2);
	}

	(*output_stDeviation) = sqrt(powSum / (this->sensors[sensorId].sampleCounter - 1));

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->sensors[sensorId].semaphore);

#ifdef DEBUG_SENSOR_MGR
	ESP_LOGI(LOG_HEADER,"standard deviation calculated: %d, sensor ID: %d, samples: %d ", (*output_stDeviation), sensorId, this->sensors[sensorId].sampleCounter);
#endif

	return  CUBEE_ERROR_OK;
}

CubeeErrorCode_t SensorMgr_resetSensor(SensorMgr_t* this, SensorMgr_SensorId_t sensorId)
{
	if((this == NULL) || (sensorId > SENSOR_MGR_ID_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reseting sensor samples");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->sensors[sensorId].semaphore, SENSOR_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking sensor semaphore, sensor ID; %d", sensorId);
		return CUBEE_ERROR_TIMEOUT;
	}

	/* Clean the sensor buffer and sample counter*/
	this->sensors[sensorId].sampleCounter = 0;
    memset(this->sensors[sensorId].dataBuffer,0,SENSOR_BUFFER_SIZE);

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->sensors[sensorId].semaphore);

	ESP_LOGI(LOG_HEADER,"Sensor data successfully, sensor ID: %d ", sensorId);

	return  CUBEE_ERROR_OK;
}

CubeeErrorCode_t SensorMgr_printSensorBuffer(SensorMgr_t* this, SensorMgr_SensorId_t sensorId)
{
	if((this == NULL) || (sensorId > SENSOR_MGR_ID_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading sensor buffer");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->sensors[sensorId].semaphore, SENSOR_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking sensor semaphore, sensor ID; %d", sensorId);
		return CUBEE_ERROR_TIMEOUT;
	}

	/* sum of stored samples */
	for(uint16_t sampleIndex = 0; sampleIndex < this->sensors[sensorId].sampleCounter; sampleIndex++)
	{
		ESP_LOGI(LOG_HEADER,"Sensor ID: %d, sample value: %d", sensorId, this->sensors[sensorId].dataBuffer[sampleIndex]);
	}

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->sensors[sensorId].semaphore);

	return  CUBEE_ERROR_OK;
}

/************************************************************************************************/
/* Internal Functions Implementation */
/************************************************************************************************/

/* Task to be created. */
void __SensorMgr_taskCode( void * pvParameters )
{
   SensorMgr_t * powerMgr = (SensorMgr_t *) pvParameters;

    for( ;; )
    {
        /* Stimulates the state machine. */
    	__SensorMgr_STMStimulate(powerMgr, &(powerMgr->stm), SENSOR_MGR_SIGNALS_NOSIG);

		/* Block for 500ms. */
		 vTaskDelay( SENSOR_MGR_TASK_DELAY );
    }
}

/* Handles the state machine */
bool __SensorMgr_STMStimulate(SensorMgr_t* this, SensorMgr_STM_t * stm, SensorMgr_Signals_t msg)
{
    bool evConsumed = false;

    switch (stm->mainState.activeSubState)
    {

    case SENSOR_MGR__INITIALIZATION:
    	/* Verify transition conditions */
    	if(SensorMgr_isInitialized(this))
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		stm->mainState.activeSubState = SENSOR_MGR__IDLE;
    		ESP_LOGI(LOG_HEADER,"Transition, Initialization -> Idle , task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());;
    	}
    	break;

    case SENSOR_MGR__IDLE:
    	for(uint8_t sensorId = 0; sensorId < SENSOR_MGR_ID_MAX; sensorId ++)
    	{
        	/* Verify transition conditions */
        	if(__SensorMgr_isMeasuringTime(this, sensorId))
        	{
        		evConsumed = true;
        		/* transition actions */
        		this->sensors[sensorId].measuringFunction(this);

        		/* transition */
        	}
    	}

        break;
    /* Insert State Machine Code */

    default:
        break;
    }

    return evConsumed;
}


/* Initialization code for this state machine */
void __SensorMgr_STM_init(SensorMgr_t* this, SensorMgr_STM_t * stm)
{
    /* State: Idle */
    stm->Idle.activeSubState = SENSOR_MGR__NOSTATE;
    /* State: Initialization */
    stm->Initialization.activeSubState = SENSOR_MGR__NOSTATE;
    /* Initial -> Initialization */
    stm->mainState.activeSubState = SENSOR_MGR__INITIALIZATION;
}


/************************************************************************************************/
/**************************** State Machine Conditions  Implementation***************************/
/************************************************************************************************/

bool __SensorMgr_isMeasuringTime(SensorMgr_t* this, SensorMgr_SensorId_t sensorId)
{
	uint64_t tick;

	if((this == NULL) || (sensorId > SENSOR_MGR_ID_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER evaluating measuring time");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Get current system tick*/
	tick = (uint64_t) xTaskGetTickCount();

	/* Debounce algorithm implementation*/
	if((tick - this->sensors[sensorId].lastSampleTime) >= this->sensors[sensorId].samplePeriod)
	{
		this->sensors[sensorId].lastSampleTime = tick;
		return true;
	}

	return false;
}


/************************************************************************************************/
/******************************* State Machine Actions  Implementation **************************/
/************************************************************************************************/

#ifdef USING_CURRENT_SENSOR
CubeeErrorCode_t __SensorMgr_measureCurrentSensor(void * this)
{
	SensorMgr_t* sensorMgr;

	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER measuring current sensor value");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	sensorMgr = (SensorMgr_t* ) this;

	if( xSemaphoreTake( sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].semaphore, SENSOR_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_TIMEOUT taking the current SENSOR semaphore");
		return CUBEE_ERROR_TIMEOUT;
	}

	if(sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].sampleCounter >= SENSOR_BUFFER_SIZE)
	{
		ESP_LOGE(LOG_HEADER,"The current sensor buffer is full, cleaning buffer");
		memset(sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].dataBuffer,0,SENSOR_BUFFER_SIZE);
		sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].sampleCounter = 0;
	}

	if(IOMgr_readCurrent(sensorMgr->ioMgr, (int16_t*) &sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].dataBuffer[sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].sampleCounter]) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error at current sensor sampling");
		/* We have finished accessing the shared resource.  Release the	 semaphore. */
		xSemaphoreGive(sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].semaphore);
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Put the measurement into the buffer */
	sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].sampleCounter++;

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(sensorMgr->sensors[SENSOR_MGR_CURRENT_SENSOR_ID].semaphore);

	return  CUBEE_ERROR_OK;
}
#endif


#ifdef USING_ACCELEROMETER
CubeeErrorCode_t __SensorMgr_measureAccelerometer(void * this)
{
	SensorMgr_t* sensorMgr;

	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER measuring accelerometer value");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	sensorMgr = (SensorMgr_t* ) this;

	if( xSemaphoreTake( sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].semaphore, SENSOR_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_TIMEOUT taking the accelerometer SENSOR semaphore");
		return CUBEE_ERROR_TIMEOUT;
	}

	if(sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].sampleCounter >= SENSOR_BUFFER_SIZE)
	{
		ESP_LOGE(LOG_HEADER,"The accelerometer sensor buffer is full, cleaning buffer");
		memset(sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].dataBuffer,0,SENSOR_BUFFER_SIZE);
		sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].sampleCounter = 0;
	}

	if(IOMgr_readAccelerometer(sensorMgr->ioMgr, &sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].dataBuffer[sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].sampleCounter]) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error at accelerometer sensor sampling");
		/* We have finished accessing the shared resource.  Release the	 semaphore. */
		xSemaphoreGive(sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].semaphore);
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Put the measurement into the buffer */
	sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].sampleCounter++;

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(sensorMgr->sensors[SENSOR_MGR_ACCELEROMETER_ID].semaphore);

	return  CUBEE_ERROR_OK;
}
#endif

#endif /* USING_SENSOR_MGR */



