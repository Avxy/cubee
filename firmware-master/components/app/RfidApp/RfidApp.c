/**
 * @file RfidApp.c
 * @version 1.0
 * @author Alex Fernandes
 * @date Sep 15, 2017
 **************************************************************************
 *
 * @brief  RFID APP
 *
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   15-Sep-2017    Alex Fernandes
 * * Original version based on
 ******************************************************************/


/* Module Dependencies */
#include "RfidApp.h"
#include "RfidApp_Cfg.h"
#include "../../../main/ProjectCfg.h"

/* FreeRTOS */

/* ESP Libraries */
#include "esp_log.h"
#include "esp_system.h"

#define LOG_HEADER 						"[RFID APP] "


#define xDEBUG_RFID_APP

#ifdef USING_RFID

/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

/**
 * @brief Task code to periodically stimulate the RFID APP state machine with the signals received in the signal queue.
 *
 * @param pvParameters : receives the RFID APP instance containing the state machine stimulated by the task
*/
static void __RfidApp_taskCode( void * pvParameters );

/**
 * @brief Function that implements the RFID APP state machine.
 *
 * @param this - pointer to RFID APP instance.
 * @param msg - Signal used to stimulate the state machine.
*/
static bool __RfidApp_STMStimulate(RfidApp_t * this, RfidApp_Signals_t msg);

/************************************************************************************************/
/* State Machine Conditions  Definition */
/************************************************************************************************/

/**
 * @brief This function verifies if the RFID card selected is a valid card.
 *
 * @param this - pointer to RFID APP instance.
 *
 * @return
 * @arg true, if the if a valid RFID card was selected.
 * @arg false, otherwise.
*/
static bool __RfidApp_STM_Condition_isValidCard(RfidApp_t * this);


/************************************************************************************************/
/* State Machine Actions  Definition */
/************************************************************************************************/

/**
 * @brief This function initializes the RFID APP state machine on INITIALIZATION state.
 *
 * @param this - pointer to RFID APP instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the state machine was successfully initialized.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __RfidApp_STM_Action_init(RfidApp_t * this);


/**
 * @brief  This function calls the RfidReader API's to read the information stored in a RFID Card.
 *
 * @param this - pointer to RFID APP instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the card information was successfully retrieved.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __RfidApp_STM_Action_readCardInfo(RfidApp_t * this);

/**
 * @brief  This function clean the information about the last RFID card selected.
 *
 * @param this - pointer to RFID APP instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the card information was successfully clean.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __RfidApp_STM_Action_resetCardInfo(RfidApp_t * this);

/**
 * @brief  This function calls the WAN Controller API to send the alarm to the server
 * indicating that a valid card was read.
 *
 * @param this - pointer to RFID APP instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the alarm was successfully send to server.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __RfidApp_STM_Action_sendCardOnAlarm(RfidApp_t * this);

/**
 * @brief  This function calls the WAN Controller API to send the alarm to the server
 * indicating the communication with a valid card is over.
 *
 * @param this - pointer to RFID APP instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the alarm was successfully send to server.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __RfidApp_STM_Action_sendCardOffAlarm(RfidApp_t * this);

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

static RfidApp_t rfidAppInstance =
{
		.initialized = false,			/*!< Indicates if the RFID application component was initialized */
		.rfidAppCfg = NULL,				/*!< Pointer to the configurations used by the RFID instance */
		.semaphore = NULL,				/*!< Semaphore used to synchronize access to RFID app resources */
		.signalQueueHandle = NULL,		/*!< Queue used to store state machine signals and stimulates the RFID app state machine*/
		.sendAlarmCallback = NULL		/*!< callback used to send RFID Alarms*/
};

static RFID_MIFARE_Key_t key =
{
	.keyByte = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}, /*!< Default authentication key to Mifare cards */
};

static StorageMgr_AlarmUnion_t alarmBuffer; 	/*!< Buffer used to save RFID alarms */


/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t RfidApp_init(RfidApp_t * this)
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	if(this == NULL)
	{
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Verify if the RFID APP is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing RFID APP... ");

	this->rfidAppCfg = RfidApp_Cfg_getInstance();
    if (RfidApp_Cfg_init(this->rfidAppCfg) != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "RFID App config initialization failed");
        return CUBEE_ERROR_UNDEFINED;
    }

	this->rfidReader = RFID_Reader_getInstance();
    if (RFID_Reader_init(this->rfidReader) != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "RFID Reader initialization failed");
        return CUBEE_ERROR_UNDEFINED;
    }

    /* Create state machine signals queue */
    this->signalQueueHandle = xQueueCreate(RFID_APP_SIGNAL_QUEUE_LENGTH, RFID_APP_SIGNAL_QUEUE_ITEM_SIZE);
	this->semaphore = xSemaphoreCreateMutex();
	if(this->semaphore == NULL)
	{
	  	ESP_LOGE(LOG_HEADER, "Error creating RFID APP semaphore");
	   	return CUBEE_ERROR_UNDEFINED;
	}

     /* Call init function of stm  for instance  */
     __RfidApp_STM_Action_init(this);

     /* Create the task, storing the handle. */
     xReturned = xTaskCreatePinnedToCore(
     					__RfidApp_taskCode,      	/* Function that implements the task. */
 						"RfidApp_task", 			/* Text name for the task. */
 						2048,      					/* Stack size in words, not bytes. */
 						( void * ) this,    		/* Parameter passed into the task. */
						RFID_APP_TASK_PRIORITY,		/* Priority at which the task is created. */
 						&xHandle,					/* Used to pass out the created task's handle. */
						RFID_APP_TASK_CORE);     	/* Specifies the core to run the task */
 	if( xReturned != pdPASS )
     {
         return CUBEE_ERROR_UNDEFINED;
     }

 	/* Build RFID key */
 	memset(&key, 0xff, MF_KEY_SIZE);

    this->initialized = true;

    ESP_LOGI(LOG_HEADER,"RFID APP successfully initialized ");

    return CUBEE_ERROR_OK;
}



RfidApp_t * RfidApp_getInstance(void)
{
	return &rfidAppInstance;
}

RfidApp_Cfg_t * RfidApp_getCfg(RfidApp_t* this)
{
	if(this == NULL)
	{
		return NULL;
	}

	return this->rfidAppCfg;
}

bool RfidApp_isInitialized(RfidApp_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

CubeeErrorCode_t RfidApp_sendSignal(RfidApp_t * this, RfidApp_Signals_t rcvSignal)
{
	RfidApp_Signals_t signal = RFID_APP_NOSIG;

	if((this == NULL) || (rcvSignal >= RFID_APP_SIGNAL_MAX))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER sending signal to RFID APP state machine");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->semaphore, RFID_APP_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking RFID APP semaphore");
		return CUBEE_ERROR_TIMEOUT;
	}

	switch(rcvSignal)
	{
	case RFID_APP_SIGNAL_INITIALIZED:
		if(this->stm.mainState.activeSubState == RFID_APP_STATE_INITIALIZATION)
		{
	    	signal = RFID_APP_SIGNAL_INITIALIZED;
		}
		break;

	case RFID_APP_SIGNAL_COMMUNICATION_FAILURE:
		if(this->stm.mainState.activeSubState == RFID_APP_STATE_CARD_VALIDATED)
		{
	    	signal = RFID_APP_SIGNAL_COMMUNICATION_FAILURE;
		}
		break;

	default:
		break;
	}

	if(signal != RFID_APP_NOSIG)
	{
		/* Send the RFID APP signal to the state machine signal queue*/
		if(xQueueGenericSend(this->signalQueueHandle, &signal, RFID_APP_SIGNAL_QUEUE_TIMEOUT, queueSEND_TO_BACK) != pdTRUE)
		{
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(this->semaphore);
			ESP_LOGE(LOG_HEADER,"Error sending signal to RFID APP state machine, signal queue is full");
			return CUBEE_ERROR_UNDEFINED;
		}
	}

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->semaphore);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t RfidApp_registerRcvCallback(RfidApp_t * this, RFIDApp_sendInstantAlarm_callback callback)
{
	if((this == NULL) || (callback == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER registering callback");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	this->sendAlarmCallback = callback;

	return CUBEE_ERROR_OK;

}


/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

static void __RfidApp_taskCode( void * pvParameters )
{
	RfidApp_t * wifiCtrl = (RfidApp_t *) pvParameters;
	RfidApp_Signals_t rcvSignal;

    for( ;; )
    {

    	/* Get signal from queue */
    	if(xQueueReceive(wifiCtrl->signalQueueHandle,&rcvSignal,RFID_APP_SIGNAL_QUEUE_TIMEOUT) == pdTRUE)
    	{
    		/* Stimulates the state machine with signal received */
    		__RfidApp_STMStimulate(wifiCtrl, rcvSignal);
    	}
    	else
    	{
    		__RfidApp_STMStimulate(wifiCtrl, RFID_APP_NOSIG);
    	}

		/* Block for 1 second. */
		 vTaskDelay( WIFI_CTRL_TASK_DELAY );
    }
}

static bool __RfidApp_STMStimulate(RfidApp_t * this, RfidApp_Signals_t msg)
{
    bool evConsumed = false;

    switch (this->stm.mainState.activeSubState)
    {
    case RFID_APP_STATE_INITIALIZATION:
    	/* Verify transition conditions */
    	if(msg == RFID_APP_SIGNAL_INITIALIZED)
    	{
    		evConsumed = true;

    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = RFID_APP_STATE_IDLE;
    		ESP_LOGI(LOG_HEADER,"Transition, RFID_APP_STATE_INITIALIZATION -> RFID_APP_STATE_IDLE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}

    	break;

    case RFID_APP_STATE_IDLE:
    	/* Verify transition conditions */
    	if(PICC_IsNewCardPresent(this->rfidReader))
    	{
    		evConsumed = true;

    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = RFID_APP_STATE_CARD_READY;
    		ESP_LOGI(LOG_HEADER,"Transition, RFID_APP_STATE_IDLE -> RFID_APP_STATE_CARD_READY, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}
    	break;

    case RFID_APP_STATE_CARD_READY:
    	/* Verify transition conditions */
    	if(PICC_ReadCardSerial(this->rfidReader))
    	{
    		evConsumed = true;

    		/* transition actions */
    		PCD_Authenticate(this->rfidReader, PICC_CMD_MF_AUTH_KEY_A, RFID_APP_BLOCK_ADDRESS_USER_ID, &key,  &(this->rfidReader->uid));
    		__RfidApp_STM_Action_readCardInfo(this);

    		/* transition */
    		this->stm.mainState.activeSubState = RFID_APP_STATE_CARD_ACTIVE;
    		ESP_LOGI(LOG_HEADER,"Transition, RFID_APP_STATE_CARD_READY -> RFID_APP_STATE_CARD_ACTIVE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}
    	else
    	{
       		evConsumed = true;

        	/* transition actions */

        	/* transition */
       		this->stm.mainState.activeSubState = RFID_APP_STATE_IDLE;
       		ESP_LOGI(LOG_HEADER,"Transition, RFID_APP_STATE_CARD_READY -> RFID_APP_STATE_IDLE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}

        break;

    case RFID_APP_STATE_CARD_ACTIVE:
    	/* Verify transition conditions */
    	if((msg == RFID_APP_SIGNAL_COMMUNICATION_FAILURE) || (!__RfidApp_STM_Condition_isValidCard(this)))
    	{
    		evConsumed = true;

    		/* transition actions */
    		__RfidApp_STM_Action_resetCardInfo(this);

    		/* transition */
    		this->stm.mainState.activeSubState = RFID_APP_STATE_IDLE;
    		ESP_LOGI(LOG_HEADER,"Transition, RFID_APP_STATE_CARD_ACTIVE -> RFID_APP_STATE_IDLE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}
    	else
    	{
       		evConsumed = true;

        	/* transition actions */
        	__RfidApp_STM_Action_sendCardOnAlarm(this);

        	/* transition */
       		this->stm.mainState.activeSubState = RFID_APP_STATE_CARD_VALIDATED;
       		ESP_LOGI(LOG_HEADER,"Transition, RFID_APP_STATE_CARD_ACTIVE -> RFID_APP_STATE_CARD_VALIDATED, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}

        break;

    case RFID_APP_STATE_CARD_VALIDATED:
    	/* Verify transition conditions */
    	if(msg == RFID_APP_SIGNAL_COMMUNICATION_FAILURE)
    	{
    		/* transition actions */
    		__RfidApp_STM_Action_sendCardOffAlarm(this);
    		__RfidApp_STM_Action_resetCardInfo(this);

    		/* transition */
    		this->stm.mainState.activeSubState = RFID_APP_STATE_IDLE;
    		ESP_LOGI(LOG_HEADER,"Transition, RFID_APP_STATE_CARD_VALIDATED -> RFID_APP_STATE_IDLE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}
    	else
    	{
    		evConsumed = true;

    		/* transition actions */
    		__RfidApp_STM_Action_readCardInfo(this);

    		/* transition */
    	}

        break;

    default:
        break;
    }

    return evConsumed;
}

/************************************************************************************************/
/* State Machine Conditions  Implementation */
/************************************************************************************************/
static bool __RfidApp_STM_Condition_isValidCard(RfidApp_t * this)
{
	return true;
}

/************************************************************************************************/
/* State Machine Actions  Definition */
/*************************** *********************************************************************/


static CubeeErrorCode_t __RfidApp_STM_Action_init(RfidApp_t * this)
{

    /* State Initialization doesn't have substate */
    this->stm.Initialization.activeSubState = RFID_APP_NOSTATE;

    /* State idle doesn't have substate */
    this->stm.Idle.activeSubState = RFID_APP_NOSTATE;

	/* State CardReady doesn't have substate*/
    this->stm.CardReady.activeSubState = RFID_APP_NOSTATE;

    /* State connected doesn't have substate */
    this->stm.CardActive.activeSubState = RFID_APP_NOSTATE;

    /* State sleeping doesn't have substate */
    this->stm.CardValidated.activeSubState = RFID_APP_NOSTATE;

    /* Initial State -> Initialization */
    this->stm.mainState.activeSubState = RFID_APP_STATE_INITIALIZATION;
    FSM_startTime(&(this->stm.Initialization));

   	/* The RFID APP state machine was successfully started, send the RFID_APP_SIGNAL_INITIALIZED signal to the state machine signal queue*/
    RfidApp_sendSignal(this, RFID_APP_SIGNAL_INITIALIZED);

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __RfidApp_STM_Action_readCardInfo(RfidApp_t * this)
{
	uint8_t readSize = RFID_APP_BLOCK_SIZE + RFID_APP_CRC_SIZE;
	RFID_StatusCode_t rfidStatusCode;

	/* Clean block read buffer */
	memset(this->readBlock,0,RFID_APP_BLOCK_SIZE + RFID_APP_CRC_SIZE);

//	PICC_DumpToSerial(this->rfidReader, &(this->rfidReader->uid));


	rfidStatusCode = MIFARE_Read(this->rfidReader,RFID_APP_BLOCK_ADDRESS_USER_ID,this->readBlock,&readSize);
	if(rfidStatusCode != STATUS_OK)
	{
		RfidApp_sendSignal(this, RFID_APP_SIGNAL_COMMUNICATION_FAILURE);
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef xDEBUG_RFID_APP
	ESP_LOGI(LOG_HEADER,"Card info read:");
	for (uint8_t index = 0; index < RFID_APP_BLOCK_SIZE; index++) {
		printf("%02x ",this->readBlock[index]);
	}
	printf("\r\n");
#endif

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __RfidApp_STM_Action_resetCardInfo(RfidApp_t * this)
{
	/* Clean block read buffer */
	memset(this->readBlock,0,RFID_APP_BLOCK_SIZE + RFID_APP_CRC_SIZE);

	/* Change the state of RFID card from ACTIVE to HALT */
	PICC_HaltA(this->rfidReader);
	PCD_StopCrypto1(this->rfidReader);

	return CUBEE_ERROR_OK;
}


static CubeeErrorCode_t __RfidApp_STM_Action_sendCardOnAlarm(RfidApp_t * this)
{
	/* Generates initialization Alarm */
	memset(&alarmBuffer.alarmArray,0,sizeof(StorageMgr_AlarmUnion_t));
	alarmBuffer.alarm.code = CUBEE_ALARM_RFID;

	if((this->sendAlarmCallback != NULL) && (this->sendAlarmCallback(&alarmBuffer.alarm) == CUBEE_ERROR_OK))
	{
		ESP_LOGI(LOG_HEADER, "Card ON alarm successfully sent");
		return CUBEE_ERROR_OK;
	}
	else if(StorageMgr_saveAlarm(this->storageMgr, &alarmBuffer) == CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Card ON alarm successfully saved in flash memory");
		return CUBEE_ERROR_OK;
	}
	else
	{
		ESP_LOGE(LOG_HEADER, "Failure sending Card ON alarm");
		return CUBEE_ERROR_UNDEFINED;
	}
}

static CubeeErrorCode_t __RfidApp_STM_Action_sendCardOffAlarm(RfidApp_t * this)
{
	/* TODO: for now it isn't necessary */

	return CUBEE_ERROR_OK;
}

#endif /* USING_RFID */



