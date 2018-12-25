/**
 * @file DB9App.h
 * @version 1.0
 * @author Alex Fernandes
 * @date Sep 29, 2017
 **************************************************************************
 *
 * @brief  DB9 APP
 *
 * This application module is responsible for execution of rules and commands DB9.
 * The application has a DB9 state machine that model the suited behavioral and
 * constraints to DB9 rules and commands.
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   29-Sep-2017    Alex Fernandes
 * * Original version
 ******************************************************************/



/* Module Dependencies */
#include "DB9App.h"
#include "DB9App_Cfg.h"
#include "../../../main/ProjectCfg.h"

/* FreeRTOS */

/* ESP Libraries */
#include "esp_log.h"
#include "esp_system.h"

#define LOG_HEADER 						"[DB9 APP] "
#define xDEBUG_DB9_APP
#ifdef USING_DB9

/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

/**
 * @brief Task code to periodically stimulate the module state machine with the signals received in the signal queue.
 *
 * @param pvParameters : receives the module instance containing the state machine stimulated by the task
*/
static void __DB9App_taskCode( void * pvParameters );

/**
 * @brief Function that implements the module state machine.
 *
 * @param this - pointer to module instance.
 * @param msg - Signal used to stimulate the state machine.
*/
static bool __DB9App_STMStimulate(DB9App_t * this, DB9App_Signals_t msg);

/************************************************************************************************/
/* State Machine Conditions  Definition */
/************************************************************************************************/

/**
 * @brief This function verifies if a new DB9 rule was received
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg true, if the if a new DB9 rule was received.
 * @arg false, otherwise.
*/
static bool __DB9App_STM_Condition_hasNewDB9Rule(DB9App_t * this);


/**
 * @brief This function verifies if there is any DB9 rule to be executed
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg true, if there is DB9 rule.
 * @arg false, otherwise.
*/
static bool __DB9App_STM_Condition_hasDB9Rule(DB9App_t * this);

/**
 * @brief This function verifies if all the rules states was already executed
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg true, if all the rules states was already executed.
 * @arg false, otherwise.
*/
static bool __DB9App_STM_Condition_hasNextState(DB9App_t * this);

/**
 * @brief This function verifies if the rules execution engine is activated
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg true, if the rule execution is activated.
 * @arg false, otherwise.
*/
static bool __DB9App_STM_Condition_isRuleExecutionEnable(DB9App_t * this);

/**
 * @brief This function evaluates if the duration defined for the execution of a DB9 rule state was reached.
 *
 * @param this - pointer to module instance.
 *
 * @arg true, if the state execution interval is bigger than rule state duration.
 * @arg false, otherwise.
*/
static bool __DB9App_STM_Condition_stateTimeout(DB9App_t * this);


/************************************************************************************************/
/* State Machine Actions  Definition */
/************************************************************************************************/

/**
 * @brief This function initializes the module state machine on INITIALIZATION state.
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the state machine was successfully initialized.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __DB9App_STM_Action_init(DB9App_t * this);

/**
 * @brief This function update the active DB9 rule with the new rule received.
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the DB9 rule was successfully updated.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __DB9App_STM_Action_updateDB9Rule(DB9App_t * this);

/**
 * @brief This function deactivate all DB9 outputs
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the DB9 outputs was successfully deactivated.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __DB9App_STM_Action_resetDB9Output(DB9App_t * this);

/**
 * @brief This function set the DB9 output based on the value received;
 *
 * The state of each bit is defined upon the respective value bit as follow:
 * BIT0 - 0 -> output DB9-0 deactivated, 1 -> DB9-0 activated
 * BIT1 - 0 -> output DB9-1 deactivated, 1 -> DB9-1 activated
 * BIT2 - 0 -> output DB9-2 deactivated, 1 -> DB9-2 activated
 * BIT3 - 0 -> output DB9-3 deactivated, 1 -> DB9-3 activated
 * BIT4 - 0 -> output DB9-4 deactivated, 1 -> DB9-4 activated
 * BIT5 - 0 -> output DB9-5 deactivated, 1 -> DB9-5 activated
 * BIT6 - 0 -> output DB9-6 deactivated, 1 -> DB9-6 activated
 * BIT7 - 0 -> output DB9-7 deactivated, 1 -> DB9-7 activated
 *
 * @param this - pointer to module instance.
 * @param value - defines the DB9 output value.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the DB9 outputs was successfully configured.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __DB9App_STM_Action_setDB9Output(DB9App_t * this, uint8_t value);

/**
 * @brief Configures the DB9 outputs and duration based on the parameters present
 * in the next state of the rule being executed, and update the state in execution.
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the DB9 state was successfully configured.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __DB9App_STM_Action_updateExecutionState(DB9App_t * this);

/**
 * @brief Clean all the information about the state in execution
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the information about the rule state in execution was successfully cleaned.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __DB9App_STM_Action_resetExecutionState(DB9App_t * this);



/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

static DB9App_t db9AppInstance =
{
		.initialized = false,			/*!< Indicates if the module was initialized */
		.db9AppCfg = NULL,				/*!< Pointer to the configurations used by the module instance */
		.semaphore = NULL,				/*!< Semaphore used to synchronize access to module resources */
		.signalQueueHandle = NULL,		/*!< Queue used to store state machine signals and stimulates the module state machine*/
		.ioMgr = NULL,					/*!< Pointer to the IO Mgr instance used by the DB9 App module */
		.newRuleQueueHandler = NULL,	/*!< Queue used to store a new rule and update the rule in execution */
		.ruleExecutionEnable = false,	/*!< Indicates if the rule execution mechanism is enabled or disabled */
		.stateExecutionCounter = -1,	/*!< Indicates the rule state in execution, -1 indicate none rule state in execution */
		.hasDB9Rule = false,			/*!< Indicates the module has a rule to be executed */
};

/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t DB9App_init(DB9App_t * this)
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;
	DB9App_rule_t db9Rule;
	StorageMgr_DB9rule_t strMgrDb9Rule;

	if(this == NULL)
	{
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Verify if the RFID APP is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing DB9 APP... ");

	this->db9AppCfg = DB9App_Cfg_getInstance();
    if (DB9App_Cfg_init(this->db9AppCfg) != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "DB9 App config initialization failed");
        return CUBEE_ERROR_UNDEFINED;
    }

    /* Create state machine signals queue */
    this->signalQueueHandle = xQueueCreate(DB9_APP_SIGNAL_QUEUE_LENGTH, DB9_APP_SIGNAL_QUEUE_ITEM_SIZE);

    /* Create DB9 new rule queue */
    this->newRuleQueueHandler = xQueueCreate(DB9_APP_RULE_QUEUE_LENGTH, DB9_APP_RULE_QUEUE_ITEM_SIZE);

    /* Create semaphore to access DB9 app shared resources */
	this->semaphore = xSemaphoreCreateMutex();
	if(this->semaphore == NULL)
	{
	  	ESP_LOGE(LOG_HEADER, "Error creating module semaphore");
	   	return CUBEE_ERROR_UNDEFINED;
	}

     /* Call init function of stm  for instance  */
     __DB9App_STM_Action_init(this);

     /* Create the task, storing the handle. */
     xReturned = xTaskCreatePinnedToCore(
     					__DB9App_taskCode,      	/* Function that implements the task. */
 						"DB9App_task", 				/* Text name for the task. */
 						2048,      					/* Stack size in words, not bytes. */
 						( void * ) this,    		/* Parameter passed into the task. */
						DB9_APP_TASK_PRIORITY,		/* Priority at which the task is created. */
 						&xHandle,					/* Used to pass out the created task's handle. */
						DB9_APP_TASK_CORE);     	/* Specifies the core to run the task */
 	if( xReturned != pdPASS )
     {
         return CUBEE_ERROR_UNDEFINED;
     }

 	/* Look for a valid rule stored in flash memory */
 	if((StorageMgr_readDB9Rule(this->storageMgr, &strMgrDb9Rule) == CUBEE_ERROR_OK) && strMgrDb9Rule.valid)
 	{
 		db9Rule.numberOfStates = strMgrDb9Rule.numberOfStates;
 		memmove(db9Rule.ruleStates,strMgrDb9Rule.ruleStates, DB9_RULE_STATE_MAX * sizeof(DB9App_ruleState_t));
 		DB9App_sendDB9Rule(this,&db9Rule);
 	}

    this->initialized = true;

    ESP_LOGI(LOG_HEADER,"DB9 APP successfully initialized ");

    return CUBEE_ERROR_OK;
}



DB9App_t * DB9App_getInstance(void)
{
	return &db9AppInstance;
}

DB9App_Cfg_t * DB9App_getCfg(DB9App_t* this)
{
	if(this == NULL)
	{
		return NULL;
	}

	return this->db9AppCfg;
}

bool DB9App_isInitialized(DB9App_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

CubeeErrorCode_t DB9App_sendSignal(DB9App_t * this, DB9App_Signals_t rcvSignal)
{
	DB9App_Signals_t signal = DB9_APP_NOSIG;

	if((this == NULL) || (rcvSignal >= DB9_APP_SIGNAL_MAX))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER sending signal to module state machine");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake(this->semaphore, DB9_APP_SEMAPHORE_TIMEOUT) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking module semaphore");
		return CUBEE_ERROR_TIMEOUT;
	}

	switch(rcvSignal)
	{
	case DB9_APP_SIGNAL_INITIALIZED:
		if(this->stm.mainState.activeSubState == DB9_APP_STATE_INITIALIZATION)
		{
	    	signal = DB9_APP_SIGNAL_INITIALIZED;
		}
		break;

	case DB9_APP_SIGNAL_COMMAND:
		if(this->stm.mainState.activeSubState == DB9_APP_STATE_DEACTIVATED)
		{
	    	signal = DB9_APP_SIGNAL_COMMAND;
		}
		break;

	default:
		break;
	}

	if(signal != DB9_APP_NOSIG)
	{
		/* Send the RFID APP signal to the state machine signal queue*/
		if(xQueueGenericSend(this->signalQueueHandle, &signal, DB9_APP_SIGNAL_QUEUE_TIMEOUT, queueSEND_TO_BACK) != pdTRUE)
		{
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(this->semaphore);
			ESP_LOGE(LOG_HEADER,"Error sending signal to module state machine, signal queue is full");
			return CUBEE_ERROR_UNDEFINED;
		}
	}

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->semaphore);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t DB9App_sendDB9Rule(DB9App_t * this, DB9App_rule_t * rule)
{
	StorageMgr_DB9rule_t strMgrDb9Rule;

	if((this == NULL) || (rule == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER inserting new rule in the queue");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Overwrite the rule in the queue */
	if(xQueueOverwrite(this->newRuleQueueHandler, rule) != pdTRUE)
	{
		ESP_LOGE(LOG_HEADER,"Error inserting new rule in the queue");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Update DB9 rule saved in flash memory */
	strMgrDb9Rule.valid = true;
	strMgrDb9Rule.numberOfStates = rule->numberOfStates;
	memmove(strMgrDb9Rule.ruleStates, rule->ruleStates, DB9_RULE_STATE_MAX * sizeof(DB9App_ruleState_t));
	StorageMgr_updateDB9Rule(this->storageMgr,&strMgrDb9Rule);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t DB9App_setDB9RuleExecution(DB9App_t * this, bool ruleExecutionEnable)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER enabling/disabling DB9 rule execution");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake(this->semaphore, DB9_APP_SEMAPHORE_TIMEOUT) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking module semaphore");
		return CUBEE_ERROR_TIMEOUT;
	}

	this->ruleExecutionEnable = ruleExecutionEnable;

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->semaphore);

	return CUBEE_ERROR_OK;
}

/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

static void __DB9App_taskCode( void * pvParameters )
{
	DB9App_t * wifiCtrl = (DB9App_t *) pvParameters;
	DB9App_Signals_t rcvSignal;

    for( ;; )
    {

    	/* Get signal from queue */
    	if(xQueueReceive(wifiCtrl->signalQueueHandle,&rcvSignal,DB9_APP_SIGNAL_QUEUE_TIMEOUT) == pdTRUE)
    	{
    		/* Stimulates the state machine with signal received */
    		__DB9App_STMStimulate(wifiCtrl, rcvSignal);
    	}
    	else
    	{
    		__DB9App_STMStimulate(wifiCtrl, DB9_APP_NOSIG);
    	}

		/* Block for 1 second. */
		 vTaskDelay( WIFI_CTRL_TASK_DELAY );
    }
}

static bool __DB9App_STMStimulate(DB9App_t * this, DB9App_Signals_t msg)
{
    bool evConsumed = false;

    switch (this->stm.mainState.activeSubState)
    {
    case DB9_APP_STATE_INITIALIZATION:
    	/* Verify transition conditions */
    	if(msg == DB9_APP_SIGNAL_INITIALIZED)
    	{
    		evConsumed = true;

    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = DB9_APP_STATE_IDLE;
#ifdef DEBUG_DB9_APP
    		ESP_LOGI(LOG_HEADER,"Transition, DB9_APP_STATE_INITIALIZATION -> DB9_APP_STATE_IDLE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_DB9_APP */
    	}

    	break;

    case DB9_APP_STATE_IDLE:
    	/* Verify transition conditions */
    	if(__DB9App_STM_Condition_hasNewDB9Rule(this))
    	{
    		evConsumed = true;

    		/* transition actions */
    		__DB9App_STM_Action_updateDB9Rule(this);

    		/* transition */
#ifdef DEBUG_DB9_APP
    		ESP_LOGI(LOG_HEADER,"Transition, DB9_APP_STATE_IDLE -> DB9_APP_STATE_IDLE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_DB9_APP */
    	}
    	else if(!__DB9App_STM_Condition_isRuleExecutionEnable(this))
    	{
    		evConsumed = true;

    		/* transition actions */
    		__DB9App_STM_Action_resetDB9Output(this);

    		/* transition */
    		this->stm.mainState.activeSubState = DB9_APP_STATE_DEACTIVATED;
#ifdef DEBUG_DB9_APP
    		ESP_LOGI(LOG_HEADER,"Transition, DB9_APP_STATE_IDLE -> DB9_APP_STATE_DEACTIVATED, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_DB9_APP */
    	}
    	else if(__DB9App_STM_Condition_hasDB9Rule(this))
    	{
    		evConsumed = true;

    		/* transition actions */
    		__DB9App_STM_Action_updateExecutionState(this);

    		/* transition */
    		this->stm.mainState.activeSubState = DB9_APP_STATE_RUNNING_RULE;
#ifdef DEBUG_DB9_APP
    		ESP_LOGI(LOG_HEADER,"Transition, DB9_APP_STATE_IDLE -> DB9_APP_STATE_RUNNING_RULE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_DB9_APP */
    	}
    	break;

    case DB9_APP_STATE_DEACTIVATED:
    	/* Verify transition conditions */
    	if(msg == DB9_APP_SIGNAL_COMMAND)
    	{
    		evConsumed = true;

    		/* transition actions */
    		__DB9App_STM_Action_setDB9Output(this,this->db9Command.value);

    		/* transition */
#ifdef DEBUG_DB9_APP
    		ESP_LOGI(LOG_HEADER,"Transition, DB9_APP_STATE_DEACTIVATED -> DB9_APP_STATE_DEACTIVATED, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_DB9_APP */

    	}
    	else if(__DB9App_STM_Condition_isRuleExecutionEnable(this))
    	{
       		evConsumed = true;

        	/* transition actions */
       		__DB9App_STM_Action_resetDB9Output(this);

        	/* transition */
       		this->stm.mainState.activeSubState = DB9_APP_STATE_IDLE;
#ifdef DEBUG_DB9_APP
       		ESP_LOGI(LOG_HEADER,"Transition, DB9_APP_STATE_DEACTIVATED -> DB9_APP_STATE_IDLE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_DB9_APP */
    	}

        break;

    case DB9_APP_STATE_RUNNING_RULE:
    	/* Verify transition conditions */
    	if((__DB9App_STM_Condition_stateTimeout(this)) && (__DB9App_STM_Condition_hasNextState(this)))
    	{
    		evConsumed = true;

    		/* transition actions */
    		__DB9App_STM_Action_updateExecutionState(this);

    		/* transition */
#ifdef DEBUG_DB9_APP
    		ESP_LOGI(LOG_HEADER,"Transition, DB9_APP_STATE_RUNNING_RULE -> DB9_APP_STATE_RUNNING_RULE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_DB9_APP */
    	}
    	else if((__DB9App_STM_Condition_stateTimeout(this)) && (!__DB9App_STM_Condition_hasNextState(this)))
    	{
       		evConsumed = true;

        	/* transition actions */
        	__DB9App_STM_Action_resetExecutionState(this);

        	/* transition */
       		this->stm.mainState.activeSubState = DB9_APP_STATE_IDLE;
#ifdef DEBUG_DB9_APP
       		ESP_LOGI(LOG_HEADER,"Transition, DB9_APP_STATE_RUNNING_RULE -> DB9_APP_STATE_IDLE, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_DB9_APP */
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

static bool __DB9App_STM_Condition_hasNewDB9Rule(DB9App_t * this)
{
	bool hasNewDB9Rule;
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER looking for new rule");
		return false;
	}

	hasNewDB9Rule = (xQueueReceive(this->newRuleQueueHandler,&this->newRule,DB9_APP_RULE_QUEUE_TIMEOUT) == pdTRUE);
	if(hasNewDB9Rule)
	{
		this->hasDB9Rule = true;
	}

	return hasNewDB9Rule;
}

static bool __DB9App_STM_Condition_hasDB9Rule(DB9App_t * this)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER checking rule");
		return false;
	}

	return this->hasDB9Rule;
}

static bool __DB9App_STM_Condition_hasNextState(DB9App_t * this)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER looking checking the next DB9 rule state");
		return false;
	}

	return (this->currentRule.numberOfStates > (this->stateExecutionCounter + 1));
}

static bool __DB9App_STM_Condition_isRuleExecutionEnable(DB9App_t * this)
{
	bool isEnabled;

	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER if DB9 rule execution is enabled");
		return false;
	}

	if( xSemaphoreTake(this->semaphore, DB9_APP_SEMAPHORE_TIMEOUT) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking module semaphore");
		return false;
	}

	isEnabled = this->ruleExecutionEnable;

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->semaphore);

	return isEnabled;
}

static bool __DB9App_STM_Condition_stateTimeout(DB9App_t * this)
{
	uint64_t timeElapsed = FSM_getTimeElapsedasd(&(this->stm.RunninRule), TIME_MILLISECONDS);

	return (timeElapsed >= this->currentRule.ruleStates[this->stateExecutionCounter].timeDuration * 1000);
}

/************************************************************************************************/
/* State Machine Actions  Definition */
/*************************** *********************************************************************/


static CubeeErrorCode_t __DB9App_STM_Action_init(DB9App_t * this)
{

    /* State Initialization doesn't have substate */
    this->stm.Initialization.activeSubState = DB9_APP_NOSTATE;

    /* State idle doesn't have substate */
    this->stm.Idle.activeSubState = DB9_APP_NOSTATE;

	/* State CardReady doesn't have substate*/
    this->stm.Deactivated.activeSubState = DB9_APP_NOSTATE;

    /* State connected doesn't have substate */
    this->stm.RunninRule.activeSubState = DB9_APP_NOSTATE;

    /* Initial State -> Initialization */
    this->stm.mainState.activeSubState = DB9_APP_STATE_INITIALIZATION;
    FSM_startTime(&(this->stm.Initialization));

   	/* The module state machine was successfully started, send the DB9_APP_SIGNAL_INITIALIZED signal to the state machine signal queue*/
    DB9App_sendSignal(this, DB9_APP_SIGNAL_INITIALIZED);

	return CUBEE_ERROR_OK;
}


static CubeeErrorCode_t __DB9App_STM_Action_updateDB9Rule(DB9App_t * this)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER updating DB9 rule");
		return CUBEE_ERROR_UNDEFINED;
	}

	if( xSemaphoreTake(this->semaphore, DB9_APP_SEMAPHORE_TIMEOUT) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking module semaphore");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Clean current rule */
	memset(&this->currentRule,0,sizeof(DB9App_rule_t));

	/* Copy the new rule received to the current rule */
	memmove(&this->currentRule,&this->newRule, sizeof(DB9App_rule_t));

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->semaphore);

	return CUBEE_ERROR_OK;
}


static CubeeErrorCode_t __DB9App_STM_Action_resetDB9Output(DB9App_t * this)
{
	return IOMgr_setDB9Output(this->ioMgr, 0);
}

static CubeeErrorCode_t __DB9App_STM_Action_setDB9Output(DB9App_t * this, uint8_t value)
{
	return IOMgr_setDB9Output(this->ioMgr, value);
}


static CubeeErrorCode_t __DB9App_STM_Action_updateExecutionState(DB9App_t * this)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER updating the execution state of the DB9 rule");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* update the state in execution */
	this->stateExecutionCounter += 1;

	/* Restart the timer for rule execution */
	FSM_startTime(&(this->stm.RunninRule));

	__DB9App_STM_Action_setDB9Output(this, this->currentRule.ruleStates[this->stateExecutionCounter].value);

	return CUBEE_ERROR_OK;
}


static CubeeErrorCode_t __DB9App_STM_Action_resetExecutionState(DB9App_t * this)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER reseting the execution state of the DB9 rule");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Restart the timer for rule execution */
	FSM_startTime(&(this->stm.RunninRule));

	/* update the state in execution */
	this->stateExecutionCounter = -1;

	return CUBEE_ERROR_OK;
}

#endif /* USING_DB9 */
