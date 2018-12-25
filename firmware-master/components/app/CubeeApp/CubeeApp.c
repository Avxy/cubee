
#include "CubeeApp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "cJSON.h"

#include "../../svc/WANController/WAN_Ctrl_Cfg.h"
#include "../../svc/StorageMgr/StorageMgr.h"

#define LOG_HEADER 									"[CUBEE_APP] "

#define	xDEBUG_CUBEE_APP


/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/
static CubeeApp_t cubeeAppInstance =
{
		.initialized = false,						/*!< Indicates if the CubeeApp instance is initialized */
		.cubeeState = false,						/*!< Keeps the current cubee activation status: true - CUBEE Activated; false - CUBEE Deactivated */
		.sendState = true,							/*!< Indicates if the CubeeApp shall send state over network */
		.ioMgr = NULL,								/*!< Pointer to the IO Mgr */
		.cubeeAppCfg = NULL,						/*!< Pointer to the configurations used by the CubeeApp instance */
		.panControler = NULL,						/*!< Pointer to the PAN Controller instance used by the CubeeApp instance */
		.wanControler = NULL,						/*!< Pointer to the WAN Controller instance used by the CubeeApp instance */
		.storageMgr = NULL,							/*!< Pointer to the StorageMgr instance used by the CubeeApp instance */
#ifdef USING_SENSOR_MGR
		.sensorMgr = NULL,							/*!< Pointer to the PowerSensor instance used by the CubeeApp instance */
#ifdef USING_ACCELEROMETER
		.sensorId = SENSOR_MGR_ACCELEROMETER_ID,	/*!< Id of sensor used by the application */
#endif /* USING_ACCELEROMETER */
#ifdef USING_CURRENT_SENSOR
		.sensorId = SENSOR_MGR_CURRENT_SENSOR_ID,	/*!< Id of sensor used by the application */
#endif /* USING_CURRENT_SENSOR */
#endif /* USING_SENSOR_MGR */
#ifdef USING_RFID
		.rfidAPP = NULL,							/*!< Pointer to the RFID APP instance used by the CubeeApp instance */
#endif /* USING_RFID */
#ifdef USING_DB9
		.db9App = NULL,								/*!< Pointer to the DB9 APP instance used by the CubeeApp instance */
#endif /* USING_DB9 */

};

/*  Queue used to store instant alarms */
QueueHandle_t instantAlarmsQueue;

/*  Buffers used to send and receive WAN and PAN data */
static WAN_Ctrl_Data_Packet_t wanDataPacket;
static uint8_t panBuffer[PAN_BUFFER_SIZE];

/* Buffer used to update the paths for send and receive data over the WAn network */
static char newPath[WAN_CTRL_MAX_DATA_PATH_SIZE];

/* Keeps the last time of WAn notification */
static uint64_t	sendNotificationTime = 0;

/* Buffer to read and update CUBEE configuration */
static CubeeConfig_t cubeeCfgBuffer =
{
		.idCubee = {0x00},				/*!< Unique CUBEE identifier */
		.cubeeName = {0x00},			/*!< CUBEE name */
		.wifiSSID = {0x00},				/*!< Wifi SSID used to connected with an Wifi access point */
		.wifiPassword = {0x00},		/*!< Wifi password used to connected with an Wifi access point */
};

#ifdef USING_SENSOR_MGR
/* Buffer to temporally store sensor sample */
static StorageMgr_SampleUnion_t sampleBuffer;
#endif/* USING_SENSOR_MGR */

/* Cubee alarm buffer */
static StorageMgr_AlarmUnion_t alarmBuffer;

/*  Semaphore to synchronize access to PAN and WAN resources */
static SemaphoreHandle_t wanSemaphore = NULL;
static SemaphoreHandle_t panSemaphore = NULL;

#ifdef USING_DB9
/* Buffer to receive a DB9 rule or command */
static DB9App_rule_t db9RuleBuffer;
#endif /* USING_DB9 */

/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/

/**
 * @brief Task code to periodically stimulate the module state machine.
 *
 * @param pvParameters : receives the module instance containing the state machine stimulated by the task
*/
static void __CubeeApp_taskCode( void * pvParameters );

/**
 * @brief Function that implements the module state machine.
 *
 * @param this - pointer to module instance.
 * @param msg - Signal used to stimulate the state machine.
*/
static bool __CubeeApp_STMStimulate(CubeeApp_t* this, CubeeApp_Signals_t msg);

/**
 * @brief This function initializes the module state machine on INITIALIZATION state.
 *
 * @param this - pointer to module instance.
 *
*/
static void __CubeeApp_STM_init(CubeeApp_t* this);

/**
 * @brief This function parse a json configuration string to a CUBEE configuration struct
 *
 * @param this - module instance
 * @param cubeeCfg - output pointer to store CUBEE configuration parsed
 * @param cubeeCfgJsonString - input configuration string
 *
 * @return
 * @arg CUBEE_ERROR_OK - The configuration was successfully parsed
 * @arg CUBEE_ERROR_UNDEFINED - Otherwise
 */
static CubeeErrorCode_t __CubeeApp_parseCubeeCfg(CubeeApp_t* this, CubeeConfig_t* cubeeCfg, uint8_t * cubeeCfgJsonString);

#ifdef USING_DB9
/**
 * @brief This function parse a DB9 rule JSON to a DB9 rule struct
 *
 * @param this - POinter to module instance
 * @param db9Rule - output pointer to store DB9 rule parsed
 * @param db9RuleJson - Input DB9 rule JSON
 *
 * @return
 * @arg CUBEE_ERROR_OK - The DB9 rule JSON was successfully parsed
 * @arg CUBEE_ERROR_UNDEFINED - Otherwise
 */
static CubeeErrorCode_t __CubeeApp_parseDB9Rule(CubeeApp_t* this, DB9App_rule_t* db9Rule, cJSON * db9RuleJson);
#endif /* USING_DB9 */

/**
 * @brief This function updates the configuration of WAN Network and all its dependencies
 *
 * @param this - Pointer to module instance
 * @param cubeeCfg - New CUBEE configuration
 *
 * @return
 * @arg CUBEE_ERROR_OK - If the WAn configuration was successfully updated
 * @arg CUBEE_ERROR_UNDEFINED - Otherwise
 */
static CubeeErrorCode_t __CubeeApp_updateWANConfig(CubeeApp_t* this, CubeeConfig_t* cubeeCfg);

/**
 * @brief This function wait timeout seconds for a command from WAN network
 *
 * @param this - Pointer do module instance
 * @param command - indicates the command to wait for.
 * @param timeout - Max time to wait.
 *
 * @return
 * @arg CUBEE_ERROR_OK - The command was received
 * @arg CUBEE_ERROR_UNDEFINED - The command wasn't received in timeout fixed
 */
static CubeeErrorCode_t __CubeeApp_waitforWANcommand(CubeeApp_t* this, CubeeApp_RcvCommands_t command, uint8_t timeout);

/**
* @brief This function notifies the CUBEE state (ACTIVATED/DEACTIVVATED) to PAN and WAn Network
*
* @param this - pointer to the CubeeApp instance.
* @param statusCode - state to send (ACTIVATE/DEACTIVATE)
*
* @return
* @arg CUBEE_ERROR_OK, if the status was successfully send over the PAN and WAN Network.
* @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __CubeeApp_sendStatus(CubeeApp_t* this, uint8_t statusCode);

/**
* @brief This function sends a command over the PAN and WAn Network
*
* @param this - pointer to the CubeeApp instance.
* @param cmd - command to send
*
* @return
* @arg CUBEE_ERROR_OK, if the command was successfully send over the PAN and WAN Network.
* @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __CubeeApp_sendCommand(CubeeApp_t* this, CubeeApp_SndCommands_t cmd);

/************************************************************************************************/
/* State Machine Conditions  Definition */
/************************************************************************************************/

/**
 * @brief This function verifies if the module instance is initilized
 *
 * @param this - Pointer to module instance
 *
 * @return
 * @arg true - If the module instance is initialized
 * @arg false - Otherwise
 */
static bool __CubeeApp_isInitialized(CubeeApp_t* this);

/**
 * @brief This function verifies if is time to send notification over WAN network
 *
 * @param this - Pointer to module instance
 *
 * @return
 * @arg true - If is time to send notification
 * @arg false - Otherwise
 */
static bool __CubeeApp_STM_Condition_sendNotificationTimeout(CubeeApp_t* this);

/************************************************************************************************/
/* State Machine Actions  Definition */
/************************************************************************************************/

/**
 * @brief This function verifies and process requests coming from PAN Network
 *
 * @param this - Pointer to module instance
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the PAN requests were successfully verified and processed
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
static CubeeErrorCode_t __CubeeApp_STM_Action_verifyPanRequest(CubeeApp_t* this);

/**
 * @brief This function verifies and process requests coming from WAN Network
 *
 * @param this - Pointer to module instance
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the WAN requests were successfully verified and processed
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
static CubeeErrorCode_t __CubeeApp_STM_Action_verifyWanRequest(CubeeApp_t* this);

/**
 * @brief This function send alarms stored in RAM memory to the server
 *
 * @param this - Pointer to module instance
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the alarms were successfully sent
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
static CubeeErrorCode_t __CubeeApp_STM_Action_sendInstantAlarms(CubeeApp_t* this);

/**
 * @brief This function sending alarms stored in flash memory over WAN Network
 *
 * @param this - Pointer to module instance
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the stored alarms were successfully sent
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
static CubeeErrorCode_t __CubeeApp_STM_Action_sendStoredAlarms(CubeeApp_t* this);

#ifdef USING_SENSOR_MGR
/**
 * @brief This function sending measurement stored in flash and RAM memory over WAN Network
 *
 * @param this - Pointer to module instance
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the measurements were successfully sent
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
static CubeeErrorCode_t __CubeeApp_STM_Action_sendMeasurement(CubeeApp_t* this);
#endif/* USING_SENSOR_MGR */

/**
 * @brief This function notifies both PAN and WAn Network about button command received
 *
 * @param this - Pointer to module instance
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the notification was successfully sent to PAN and WAn Network
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
static CubeeErrorCode_t __CubeeApp_STM_Action_sendButtonCmd(CubeeApp_t* this);

/************************************************************************************************/
/* Public API  Implementation */
/************************************************************************************************/
/* Operation init of class CubeeApp */
CubeeErrorCode_t CubeeApp_init(CubeeApp_t* this)
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;
	CubeeErrorCode_t cubeeError;

	/* Verify if the CubeeApp is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing CUBEE Application ...");

    /* Create instant alarm queue */
    instantAlarmsQueue = xQueueCreate(INSTANT_ALARM_QUEUE_LENGTH, INSTANT_ALARM_QUEUE_ITEM_SIZE);
	if(instantAlarmsQueue == NULL)
	{
	  	ESP_LOGE(LOG_HEADER, "Error creating instant alarm queue");
	   	return CUBEE_ERROR_UNDEFINED;
	}

	/* Init CubeeApp configuration */
	this->cubeeAppCfg = CubeeApp_Cfg_getInstance();
	cubeeError = CubeeApp_Cfg_init(this->cubeeAppCfg);
    if (cubeeError != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "CUBEE App config initialization failed");
        esp_restart();
    }

	/* Init PAN service */
	this->panControler = PANController_getInstance();
    if (PANController_init(this->panControler) != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "PAN Controller initialization failed");
        esp_restart();
    }

    /* Init WAN service */
	this->wanControler = WAN_Ctrl_getInstance();
    if (WAN_Ctrl_init(this->wanControler) != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "WAN Controller initialization failed");
        esp_restart();
    }

    /* Init IO Manager service */
	this->ioMgr = IOMgr_getInstance();
	if(IOMgr_init(this->ioMgr) != CUBEE_ERROR_OK){
		ESP_LOGE(LOG_HEADER, "IOMgr initialization failed");
		esp_restart();
	}

	/* Init Sensor Manager service */
#ifdef USING_SENSOR_MGR
	this->sensorMgr = SensorMgr_getInstance();
	if(SensorMgr_init(this->sensorMgr) != CUBEE_ERROR_OK){
		ESP_LOGE(LOG_HEADER, "PowerSensor initialization failed");
		esp_restart();
	}
#endif /* USING_SENSOR_MGR */

	this->storageMgr = StorageMgr_getInstance();
    if (StorageMgr_Init(this->storageMgr) != CUBEE_ERROR_OK)
    {
    	ESP_LOGE(LOG_HEADER, "Storage Manager initialization failed");
    	esp_restart();
    }

#ifdef USING_RFID
	this->rfidAPP = RfidApp_getInstance();
	this->rfidAPP->storageMgr = this->storageMgr;
	RfidApp_registerRcvCallback(this->rfidAPP, CubeeApp_sendInstantAlarm);
    if (RfidApp_init(this->rfidAPP) != CUBEE_ERROR_OK)
    {
    	ESP_LOGE(LOG_HEADER, "RFID APP initialization failed");
    	esp_restart();
    }

#endif /* USING_RFID */

#ifdef USING_DB9
    this->db9App = DB9App_getInstance();
	this->db9App->ioMgr = this->ioMgr;
	this->db9App->storageMgr = this->storageMgr;
	if(DB9App_init(this->db9App) != CUBEE_ERROR_OK)
	{
	  	ESP_LOGE(LOG_HEADER, "DB9 APP initialization failed");
	    esp_restart();
	}
#endif /* USING_DB9 */

    /* Create Mutex to access WAN and PAN services */
    panSemaphore = xSemaphoreCreateMutex();
    wanSemaphore =  xSemaphoreCreateMutex();
    if((wanSemaphore == NULL) || (panSemaphore == NULL))
    {
    	ESP_LOGE(LOG_HEADER, "Error creating WAN or PAN semaphore");
    	esp_restart();
    }

    /* read Cubee configuration */
    StorageMgr_readCubeeCfg(this->storageMgr,&cubeeCfgBuffer);

    /* Updating WAN configuration */
	memset(this->wanControler->idCubee, 0, ID_CUBEE_MAX_SIZE);
	memmove(this->wanControler->idCubee, cubeeCfgBuffer.idCubee, (size_t)strlen((char*)cubeeCfgBuffer.idCubee));
	if(__CubeeApp_updateWANConfig(this,&cubeeCfgBuffer) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error updating WAN configuration");
		esp_restart();
	}

	/* Update data path */
	snprintf(newPath, WAN_CTRL_MAX_DATA_PATH_SIZE, "cubee/%s/command", (char *)cubeeCfgBuffer.idCubee);
	if(WAN_Ctrl_updateRcvDataPath(this->wanControler, WAN_CTRL_RCV_DATA_COMMAND, newPath) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating receive data path");
	}

    /* Updating PAN network */
	if(PANController_setName(this->panControler, cubeeCfgBuffer.cubeeName) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error updating PAN configuration");
		esp_restart();
	}

	/* Generates initialization Alarm */
	memset(&alarmBuffer.alarmArray,0,sizeof(StorageMgr_AlarmUnion_t));
	alarmBuffer.alarm.code = CUBEE_ALARM_INITIALIZATION;
	StorageMgr_saveAlarm(this->storageMgr, &alarmBuffer);

	/* Initial state is activated */
	this->cubeeState = true;
	IOMgr_activate(this->ioMgr);
	this->sendState = true;

    /* Call init function of stm  for instance  */
    __CubeeApp_STM_init(this);

    /* Create the task, storing the handle. */
    xReturned = xTaskCreatePinnedToCore(
    					__CubeeApp_taskCode,    /* Function that implements the task. */
						"CubeeApp_task", 		/* Text name for the task. */
						4096,      				/* Stack size in words, not bytes. */
						( void * ) this,    	/* Parameter passed into the task. */
						CUBEE_APP_TASK_PRIORITY,/* Priority at which the task is created. */
						&xHandle,				/* Used to pass out the created task's handle. */
						CUBEE_APP_TASK_CORE);   /* Specifies the core to run the task */
	if( xReturned != pdPASS )
    {
		esp_restart();
    }

	this->initialized = true;

	ESP_LOGI(LOG_HEADER,"CUBEE Application initialized ");

	return CUBEE_ERROR_OK;
}

CubeeApp_t * CubeeApp_getInstance(void)
{
	return &cubeeAppInstance;
}

CubeeApp_Cfg_t * CubeeApp_getCfg(CubeeApp_t* this)
{
	if(this == NULL)
	{
		return NULL;
	}

	return this->cubeeAppCfg;
}

bool CubeeApp_isInitialized(CubeeApp_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

CubeeErrorCode_t CubeeApp_sendInstantAlarm(StorageMgr_Alarm_t *alarm)
{
	if(alarm == NULL)
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER add instant alarm to queue");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Overwrite the rule in the queue */
	if(xQueueSend(instantAlarmsQueue, alarm, INSTANT_ALARM_QUEUE_TIMEOUT) != pdTRUE)
	{
		ESP_LOGE(LOG_HEADER,"Error sending alarm to instant queue");
		return CUBEE_ERROR_TIMEOUT;
	}

	ESP_LOGI(LOG_HEADER, "Alarm successfully sent to instant queue");
	return CUBEE_ERROR_OK;
}

/************************************************************************************************/
/* Internal Functions Implementation */
/************************************************************************************************/

/* Task to be created. */
static void __CubeeApp_taskCode( void * pvParameters )
{
   CubeeApp_t * notifier = (CubeeApp_t *) pvParameters;

    for( ;; )
    {
        /* Stimulates the state machine. */
    	__CubeeApp_STMStimulate(notifier, NOSIG);

		/* Block for 500ms. */
		 vTaskDelay(CUBEE_APP_TASK_DELAY);
    }
}

/* Handles the state machine */
static bool __CubeeApp_STMStimulate(CubeeApp_t* this, CubeeApp_Signals_t msg)
{
    bool evConsumed = false;

    switch (this->stm.mainState.activeSubState)
    {

    case CubeeApp__Initialization:
    	/* Verify transition conditions */
    	if(__CubeeApp_isInitialized(this))
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = CubeeApp__Idle;
    		ESP_LOGI(LOG_HEADER,"Transition, Initialization -> Idle , task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());;
    	}
    	break;

    case CubeeApp__Idle:
    	/* Check PAN Network */
    	__CubeeApp_STM_Action_verifyPanRequest(this);

    	/* Check WAN Network */
    	__CubeeApp_STM_Action_verifyWanRequest(this);

    	/* Send instant alarms */
    	__CubeeApp_STM_Action_sendInstantAlarms(this);

    	/* Check button command */
    	if(IOMgr_hasButtonCmd(this->ioMgr))
    	{
    		evConsumed = true;
    		/* transition actions */
    		__CubeeApp_STM_Action_sendButtonCmd(this);
    	}

    	/* Check send notification timeout */
    	if(__CubeeApp_STM_Condition_sendNotificationTimeout(this))
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = CubeeApp__SendingNotification;
    		ESP_LOGI(LOG_HEADER,"Transition, Idle -> SendingNotification ");
    	}

        break;

    case CubeeApp__SendingNotification:

   		/* Send stored alarms */
    	__CubeeApp_STM_Action_sendStoredAlarms(this);

#ifdef USING_SENSOR_MGR
    	/* Send stored measurements */
    	__CubeeApp_STM_Action_sendMeasurement(this);
#endif
   		/* transition */
    	this->stm.mainState.activeSubState = CubeeApp__Idle;
    	ESP_LOGI(LOG_HEADER,"Transition, SendingNotification -> Idle ");
        break;
    default:
        break;
    }

    return evConsumed;
}


/* Initialization code for this state machine */
static void __CubeeApp_STM_init(CubeeApp_t* this)
{
    /* State: Configuration */
	this->stm.Configuration.activeSubState = CubeeApp__NOSTATE;
    /* State: Idle */
	this->stm.Idle.activeSubState = CubeeApp__NOSTATE;
    /* State: Initialization */
	this->stm.Initialization.activeSubState = CubeeApp__NOSTATE;
    /* State: SendingNotification */
	this->stm.SendingNotification.activeSubState = CubeeApp__NOSTATE;

    /* Initial -> Initialization */
	this->stm.mainState.activeSubState = CubeeApp__Initialization;
}

static CubeeErrorCode_t __CubeeApp_parseCubeeCfg(CubeeApp_t* this, CubeeConfig_t* cubeeCfg, uint8_t * cubeeCfgJsonString)
{
	cJSON *cubeeCfgJson;
	cJSON *idCubeeJson;
	cJSON *cubeeNameJson;
	cJSON *wifiSSIDJson;
	cJSON *wifiPassJson;

	if((this == NULL) || (cubeeCfg == NULL) || (cubeeCfgJsonString == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER parsing CUBEE configuration Json string.");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Clean configuration struct fields */
	memset(cubeeCfg->cubeeName,0,CUBEE_NAME_MAX_SIZE);
	memset(cubeeCfg->idCubee,0,ID_CUBEE_MAX_SIZE);
	memset(cubeeCfg->wifiPassword,0,PASSWORD_MAX_SIZE);
	memset(cubeeCfg->wifiSSID,0,SSID_MAX_SIZE);

	/* Process CUBEE configuration JSON string */
	cubeeCfgJson = cJSON_Parse((const char *) cubeeCfgJsonString);
	if(cubeeCfgJson == NULL)
	{
		ESP_LOGE(LOG_HEADER,"Error parsing CUBEE configuration Json string, invalid json format.");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
#ifdef DEBUG_CUBEE_APP
	ESP_LOGI(LOG_HEADER,"CUBEE configuration json received: %s", cJSON_PrintUnformatted(cubeeCfgJson));
#endif /*DEBUG_CUBEE_APP*/

	/* Parse CUBEE ID */
	idCubeeJson = cJSON_GetObjectItem(cubeeCfgJson, "idCubee");
	if((idCubeeJson == NULL) || (idCubeeJson->valuestring == NULL) || (strlen(idCubeeJson->valuestring) > ID_CUBEE_MAX_SIZE))
	{
		ESP_LOGE(LOG_HEADER,"Error parsing CUBEE configuration Json string, idCubee invalid ");
		return CUBEE_ERROR_UNDEFINED;
	}
	else
	{
		memmove(cubeeCfg->idCubee,idCubeeJson->valuestring, strlen(idCubeeJson->valuestring));
		ESP_LOGI(LOG_HEADER,"Parsing CUBEE configuration Json string, CUBEE ID:%s ", (char*)cubeeCfg->idCubee);
	}

	/* Parse CUBEE Name*/
	cubeeNameJson = cJSON_GetObjectItem(cubeeCfgJson, "cubeeName");
	if((cubeeNameJson == NULL) || (cubeeNameJson->valuestring == NULL) || (strlen(cubeeNameJson->valuestring) > CUBEE_NAME_MAX_SIZE))
	{
		ESP_LOGE(LOG_HEADER,"Error parsing CUBEE configuration Json string, cubeeName invalid ");
		return CUBEE_ERROR_UNDEFINED;
	}
	else
	{
		memmove(cubeeCfg->cubeeName,cubeeNameJson->valuestring, strlen(cubeeNameJson->valuestring));
		ESP_LOGI(LOG_HEADER,"Parsing CUBEE configuration Json string, CUBEE Name:%s ", (char*)cubeeCfg->cubeeName);
	}

	/* Parse wifi SSID*/
	wifiSSIDJson = cJSON_GetObjectItem(cubeeCfgJson, "wifiSSID");
	if((wifiSSIDJson == NULL) || (wifiSSIDJson->valuestring == NULL) || (strlen(wifiSSIDJson->valuestring) >SSID_MAX_SIZE))
	{
		ESP_LOGE(LOG_HEADER,"Error parsing CUBEE configuration Json string, wifi SSID invalid ");
		return CUBEE_ERROR_UNDEFINED;
	}
	else
	{
		memmove(cubeeCfg->wifiSSID,wifiSSIDJson->valuestring, strlen(wifiSSIDJson->valuestring));
		ESP_LOGI(LOG_HEADER,"Parsing CUBEE configuration Json string, Wifi SSID:%s ", (char*)cubeeCfg->wifiSSID);
	}

	/* Parse wifi password*/
	wifiPassJson = cJSON_GetObjectItem(cubeeCfgJson, "wifiPass");
	if((wifiPassJson == NULL) || (wifiPassJson->valuestring == NULL) || (strlen(wifiPassJson->valuestring) >SSID_MAX_SIZE))
	{
		ESP_LOGE(LOG_HEADER,"Error parsing CUBEE configuration Json string, wifi SSID invalid ");
		return CUBEE_ERROR_UNDEFINED;
	}
	else
	{
		memmove(cubeeCfg->wifiPassword,wifiPassJson->valuestring, strlen(wifiPassJson->valuestring));
		ESP_LOGI(LOG_HEADER,"Parsing CUBEE configuration Json string, Wifi password:%s ", (char*)cubeeCfg->wifiPassword);
	}

	return CUBEE_ERROR_OK;
}

#ifdef USING_DB9
static CubeeErrorCode_t __CubeeApp_parseDB9Rule(CubeeApp_t* this, DB9App_rule_t* db9Rule, cJSON * db9RuleJson)
{
	cJSON *ruleStateArrayJson;
	cJSON *ruleStateJson;

	if((this == NULL) || (db9Rule == NULL) || (db9RuleJson == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER parsing Json string of DB9 Rule .");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Clean DB9 rule struct */
	memset(db9Rule,0,sizeof(DB9App_rule_t));

#ifdef DEBUG_CUBEE_APP
	ESP_LOGI(LOG_HEADER,"DB9 rule, json received: %s", cJSON_PrintUnformatted(db9RuleJson));
#endif /*DEBUG_CUBEE_APP*/

	/* Parse number of states */
	db9Rule->numberOfStates = cJSON_GetObjectItem(db9RuleJson, "nStates")->valueint;
	if((db9Rule->numberOfStates == 0) || (db9Rule->numberOfStates > DB9_RULE_STATE_MAX))
	{
		ESP_LOGE(LOG_HEADER,"Error parsing CJson string of DB9 Rule, invalid number of states, %d ", db9Rule->numberOfStates);
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Parse rule states */
	ruleStateArrayJson = cJSON_GetObjectItem(db9RuleJson, "states");
	for(uint8_t stateIndex = 0; stateIndex < db9Rule->numberOfStates; stateIndex ++)
	{
		ruleStateJson = cJSON_GetArrayItem(ruleStateArrayJson,stateIndex);
		db9Rule->ruleStates[stateIndex].timeDuration = cJSON_GetObjectItem(ruleStateJson , "time")->valueint;
		db9Rule->ruleStates[stateIndex].value = cJSON_GetObjectItem(ruleStateJson, "value")->valueint;
	}

	return CUBEE_ERROR_OK;
}
#endif /* USING_DB9 */

static CubeeErrorCode_t __CubeeApp_updateWANConfig(CubeeApp_t* this, CubeeConfig_t* cubeeCfg)
{
	memset(this->wanControler->wanControllerCfg->idCubee, 0, ID_CUBEE_MAX_SIZE);
	memset(this->wanControler->wanControllerCfg->wifiPassword, 0, PASSWORD_MAX_SIZE);
	memset(this->wanControler->wanControllerCfg->wifiSSID, 0, SSID_MAX_SIZE);

#ifdef DEBUG_CUBEE_APP
	ESP_LOGI(LOG_HEADER,"New CUBEE configuration: ");
	ESP_LOGI(LOG_HEADER,"CUBEE ID: %s", (char *)cubeeCfg->idCubee);
	ESP_LOGI(LOG_HEADER,"Wifi Password: %s", (char *)cubeeCfg->wifiPassword);
	ESP_LOGI(LOG_HEADER,"Wifi SSID: %s", (char *)cubeeCfg->wifiSSID);
#endif /* #ifdef DEBUG_CUBEE_APP */

	/* Update WAN configuration fields */
	memmove(this->wanControler->wanControllerCfg->idCubee, cubeeCfg->idCubee, ID_CUBEE_MAX_SIZE);
	memmove(this->wanControler->wanControllerCfg->wifiPassword, cubeeCfg->wifiPassword, PASSWORD_MAX_SIZE);
	memmove(this->wanControler->wanControllerCfg->wifiSSID, cubeeCfg->wifiSSID, SSID_MAX_SIZE);


	if(WAN_Ctrl_updateCfg(this->wanControler) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating WAN configuration");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __CubeeApp_waitforWANcommand(CubeeApp_t* this, CubeeApp_RcvCommands_t command, uint8_t timeout)
{
	cJSON *wanJson;
	uint64_t timeNow;
	uint64_t maxTime;

	timeNow = (uint32_t) xTaskGetTickCount();
	maxTime = timeNow + ((timeout * 1000) / portTICK_PERIOD_MS);

	while(timeNow < maxTime)
	{
		memset(&wanDataPacket, 0, sizeof(WAN_Ctrl_Data_Packet_t));
		wanDataPacket.dataType = WAN_CTRL_RCV_DATA_COMMAND;
		if(WAN_Ctrl_rcvData(this->wanControler, &wanDataPacket) == CUBEE_ERROR_OK)
		{
			/* Process JSON received*/
			wanJson = cJSON_Parse((const char *) wanDataPacket.dataBuffer);
			if((wanJson != NULL) &&
				(cJSON_GetObjectItem(wanJson, "appCommand") != NULL) &&
				(cJSON_GetObjectItem(wanJson, "appCommand")->valueint == command))
			{
				ESP_LOGI(LOG_HEADER, "The wait for command %d ended successfully", command);
				return CUBEE_ERROR_OK;
			}
		}

		vTaskDelay(CUBEE_APP_TASK_DELAY);
		timeNow = (uint32_t) xTaskGetTickCount();
	}

	ESP_LOGE(LOG_HEADER, "The wait for command %d failed", command);
	return CUBEE_ERROR_UNDEFINED;
}


CubeeErrorCode_t __CubeeApp_sendStatus(CubeeApp_t* this, uint8_t statusCode)
{
	cJSON* statusJSon;
	char  * statusJSonString;
	CubeeErrorCode_t panError;
	CubeeErrorCode_t wanError;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER sending status %#02x", statusCode);
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(!this->sendState)
	{
		/* Doens't have state change to send */
		return CUBEE_ERROR_OK;
	}

#ifdef USING_DB9
	/* Update DB9 App */
	DB9App_setDB9RuleExecution(this->db9App,this->cubeeState);
#endif /* USING_DB9 */

	/* Build status JSON */
	StorageMgr_readCubeeCfg(this->storageMgr,&cubeeCfgBuffer);
	statusJSon = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(statusJSon, "idCubee", cJSON_CreateString((const char*)cubeeCfgBuffer.idCubee));
	cJSON_AddNumberToObject(statusJSon,"state",this->cubeeState);
	memset(&wanDataPacket, 0, sizeof(WAN_Ctrl_Data_Packet_t));
	statusJSonString = cJSON_PrintUnformatted(statusJSon);
	memmove(wanDataPacket.dataBuffer, (uint8_t *) statusJSonString, strlen(statusJSonString));
	wanDataPacket.dataType = WAN_CTRL_SND_DATA_STATUS;
	wanDataPacket.dataBufferSize = strlen(statusJSonString);

	/* Send status to PAN Network  */
	panError = PANController_sendBluetoothData(this->panControler, (uint8_t *) &statusCode, PAN_CONTROLLER_DATA_MONITOR_ID, sizeof(uint8_t));
	if(panError != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error sending status %#02x over PAN Network", statusCode);
	}

	/* Send command to WAN Network  */
	wanError = WAN_Ctrl_sendData(this->wanControler,&wanDataPacket, true);
	if(wanError != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error sending status %#02x over WAn Network", statusCode);
	}

	if((panError == CUBEE_ERROR_OK) && (wanError == CUBEE_ERROR_OK))
	{
		ESP_LOGI(LOG_HEADER,"Status %#02x successfully sent to WAN and PAN Network", statusCode);
		/* Disable the send state flag */
		this->sendState = false;
		return CUBEE_ERROR_OK;
	}

	/* Keeps the send state flag enable */
	this->sendState = true;

	return CUBEE_ERROR_UNDEFINED;

}

CubeeErrorCode_t __CubeeApp_sendCommand(CubeeApp_t* this, CubeeApp_SndCommands_t cmd)
{
	cJSON* commandJSon;
	char  * commandJSonString;
	CubeeErrorCode_t panError;
	CubeeErrorCode_t wanError;


	if((cmd >= CUBEE_APP_SND_CMD_MAX) || (this == NULL))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER sending command %#02x", cmd);
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Build command JSON */
	StorageMgr_readCubeeCfg(this->storageMgr,&cubeeCfgBuffer);
	commandJSon = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(commandJSon, "idCubee", cJSON_CreateString((const char*)cubeeCfgBuffer.idCubee));
	cJSON_AddNumberToObject(commandJSon,"signal",cmd);
	memset(&wanDataPacket, 0, sizeof(WAN_Ctrl_Data_Packet_t));
	commandJSonString = cJSON_PrintUnformatted(commandJSon);
	memmove(wanDataPacket.dataBuffer, (uint8_t *) commandJSonString, strlen(commandJSonString));
	wanDataPacket.dataType = WAN_CTRL_SND_DATA_COMMAND;
	wanDataPacket.dataBufferSize = strlen(commandJSonString);

	/* Send command to PAN Network  */
	panError = PANController_sendBluetoothData(this->panControler, (uint8_t *) &cmd, PAN_CONTROLLER_SND_CMD_ID, sizeof(uint8_t));
	if(panError != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error sending command %#02x over PAN Network", cmd);
	}

	/* Send command to WAN Network  */
	wanError = WAN_Ctrl_sendData(this->wanControler,&wanDataPacket, false);
	if(wanError != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error sending command %#02x over WAn Network", cmd);
	}

	if((panError == CUBEE_ERROR_OK) && (wanError == CUBEE_ERROR_OK))
	{
		ESP_LOGI(LOG_HEADER,"Command %#02x successfully sent to WAN and PAN Network", cmd);
		return CUBEE_ERROR_OK;
	}

	return CUBEE_ERROR_UNDEFINED;
}

/************************************************************************************************/
/**************************** State Machine Conditions  Implementation***************************/
/************************************************************************************************/


static bool __CubeeApp_isInitialized(CubeeApp_t* this)
{
	return (this->initialized);
}

bool __CubeeApp_STM_Condition_sendNotificationTimeout( CubeeApp_t* this)
{
	uint64_t tick;

	/* Get current system tick*/
	tick = (uint64_t) xTaskGetTickCount();

	if((tick - sendNotificationTime) >= SEND_NOTIFICATION_PERIOD)
	{
		/* Reset debounce time*/
		sendNotificationTime = tick;
		return true;
	}

	return false;
}

/************************************************************************************************/
/******************************* State Machine Actions  Implementation **************************/
/************************************************************************************************/
static CubeeErrorCode_t __CubeeApp_STM_Action_verifyPanRequest(CubeeApp_t* this)
{

	CubeeErrorCode_t error;
	uint8_t panData = 0x00;
	cJSON * cubeeCfgJson;
	cJSON * oldIdCubeeJSON;
	char * cubeeCfgJsonString;


	if( xSemaphoreTake( panSemaphore, PAN_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking the PAN semaphore");
		return CUBEE_ERROR_UNDEFINED;
	}

	error = PANController_receiveBluetoothCmd(this->panControler, (uint8_t *)&panData, sizeof(uint8_t));

	if(error == CUBEE_ERROR_TIMEOUT)
	{
		/* The PAN Network doesn't have command. */
		xSemaphoreGive(panSemaphore);
		return CUBEE_ERROR_OK;
	}

	else if((panData == CUBEE_APP_RCV_CMD_DEACTIVATE) && (this->cubeeState == true))
	{
		ESP_LOGI(LOG_HEADER,"PAN command Received, DEACTIVATE");
		if (IOMgr_deactivate(this->ioMgr) == CUBEE_ERROR_OK)
		{
			ESP_LOGI(LOG_HEADER,"CUBEE successfully deactivated");
			this->cubeeState = false;
			this->sendState = true;
		}
	}
	else if((panData == CUBEE_APP_RCV_CMD_ACTIVATE) && (this->cubeeState == false))
	{
		ESP_LOGI(LOG_HEADER," PAN command Received, ACTIVATE");
		if (IOMgr_activate(this->ioMgr) == CUBEE_ERROR_OK)
		{
			ESP_LOGI(LOG_HEADER,"CUBEE successfully activated");
			this->cubeeState = true;
			this->sendState = true;
		}
	}
	else if(panData == CUBEE_APP_RCV_CMD_LED)
	{
		ESP_LOGI(LOG_HEADER,"PAN command Received, LED");
		if(IOMgr_toogleLED(this->ioMgr) == CUBEE_ERROR_OK)
		{
			ESP_LOGI(LOG_HEADER,"LED command successfully executed");
		}
	}
#ifdef USING_DB9
	else if(panData == CUBEE_APP_RCV_CMD_DB9)
	{
		ESP_LOGI(LOG_HEADER,"PAN command Received, DB9");
		error = PANController_receiveBluetoothDB9(this->panControler,panBuffer,PAN_BUFFER_SIZE);
		if(error != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error receiving DB9 command from PAN network");
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(panSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}
		/* Send command to DB9 app */
		this->db9App->db9Command.value = panBuffer[0];
		DB9App_sendSignal(this->db9App,DB9_APP_SIGNAL_COMMAND);
	}
#endif /* USING_DB9 */
	else if (panData == CUBEE_APP_RCV_CMD_CONFIG)
	{
		ESP_LOGI(LOG_HEADER, "PAN command Received, CONFIGURATION");
		ESP_LOGI(LOG_HEADER, "Updating CUBEE configuration...");

		/* Read current cubee ID */
		memset(&cubeeCfgBuffer, 0, sizeof(CubeeConfig_t));
		StorageMgr_readCubeeCfg(this->storageMgr,&cubeeCfgBuffer);
		oldIdCubeeJSON = cJSON_CreateString((const char*)cubeeCfgBuffer.idCubee);

		/* Copy the configuration received to the PAN buffer */
		memset(panBuffer,0,PAN_BUFFER_SIZE);
		error = PANController_receiveBluetoothConfig(this->panControler,panBuffer,PAN_BUFFER_SIZE);
		if((error != CUBEE_ERROR_OK) || (strlen((char *)panBuffer) <= 0 ))
		{
			ESP_LOGE(LOG_HEADER,"Error receiving configuration from PAN network");
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(panSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}

		/* Parse the CUBEE configuration JSON received */
		memset(&cubeeCfgBuffer, 0, sizeof(CubeeConfig_t));
		if(__CubeeApp_parseCubeeCfg(this,&cubeeCfgBuffer,panBuffer) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error parsing configuration");
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(panSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}

		/* Add current cubee ID to CUBEE configuration JSON */
		cubeeCfgJson = cJSON_Parse((const char *) panBuffer);
		cJSON_AddItemToObjectCS(cubeeCfgJson, "oldIdCubee", oldIdCubeeJSON);

		if( xSemaphoreTake( wanSemaphore, WAN_SEMAPHORE_TIMEOUT ) != pdTRUE )
		{
			/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
			ESP_LOGE(LOG_HEADER,"Error taking the WAN semaphore");
			xSemaphoreGive(panSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}

		/* Updating WAN configuration */
		if(__CubeeApp_updateWANConfig(this,&cubeeCfgBuffer) != CUBEE_ERROR_OK)
		{
			xSemaphoreGive(wanSemaphore);
			xSemaphoreGive(panSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}

		/* Send CUBEE configuration to server */
		cubeeCfgJsonString = cJSON_PrintUnformatted(cubeeCfgJson);
		ESP_LOGI(LOG_HEADER, "Configuration JSON build: %s", cubeeCfgJsonString);
		memmove(wanDataPacket.dataBuffer, (uint8_t *) cubeeCfgJsonString, strlen(cubeeCfgJsonString));
		wanDataPacket.dataBufferSize = strlen(cubeeCfgJsonString);
		wanDataPacket.dataType = WAN_CTRL_SND_DATA_CONFIG;
		if(WAN_Ctrl_sendData(this->wanControler,&wanDataPacket, true) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error sending configuration to server");
			xSemaphoreGive(wanSemaphore);
			xSemaphoreGive(panSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}

		/* Wait for registration acknowledgment */
		if(__CubeeApp_waitforWANcommand(this, CUBEE_APP_RCV_CMD_REG_ACK, CUBEE_REG_ACK_TIMEOUT) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error receiving CUBEE registration acknowledgment from server");
			xSemaphoreGive(wanSemaphore);
			xSemaphoreGive(panSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}

		/* Update data path */
		snprintf(newPath, WAN_CTRL_MAX_DATA_PATH_SIZE, "cubee/%s/command", (char *)cubeeCfgBuffer.idCubee);
		while(WAN_Ctrl_updateRcvDataPath(this->wanControler, WAN_CTRL_RCV_DATA_COMMAND, newPath) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER, "Error updating receive data path");
			vTaskDelay(CUBEE_APP_TASK_DELAY);
		}

		/* Store received configuration */
		while(StorageMgr_updateCubeeCfg(this->storageMgr, &cubeeCfgBuffer) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error saving CUBEE configuration");
			vTaskDelay(CUBEE_APP_TASK_DELAY);
		}

	    /* Updating PAN network */
		if(PANController_setName(this->panControler, cubeeCfgBuffer.cubeeName) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error updating PAN configuration");
		}

		/* updates Cubee id used to WAN requests */
		memset(this->wanControler->idCubee, 0, ID_CUBEE_MAX_SIZE);
		memmove(this->wanControler->idCubee, cubeeCfgBuffer.idCubee, (size_t)strlen((char*)cubeeCfgBuffer.idCubee));

		/* Request to send state */
		this->sendState = true;

		ESP_LOGI(LOG_HEADER, "CUBEE configuration successfully updated");

	}

	/* If required, send CUBEE state to PAN and WAn network */
	if(this->sendState)
	{
		if(this->cubeeState == true)
		{
			__CubeeApp_sendStatus(this, CUBEE_APP_RCV_CMD_ACTIVATE);
		}
		else
		{
			__CubeeApp_sendStatus(this, CUBEE_APP_RCV_CMD_DEACTIVATE);
		}
	}


	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(wanSemaphore);
	xSemaphoreGive(panSemaphore);


	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __CubeeApp_STM_Action_verifyWanRequest(CubeeApp_t* this)
{
	cJSON *wanJson;
	uint8_t appCommand;
#ifdef USING_DB9
	cJSON *db9RUleJson;
#endif /* USING_DB9 */

	if( xSemaphoreTake( wanSemaphore, WAN_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking the WAN semaphore");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* If required, send CUBEE state to PAN and WAn network */
	if(this->sendState)
	{
		if(this->cubeeState == true)
		{
			__CubeeApp_sendStatus(this, CUBEE_APP_RCV_CMD_ACTIVATE);
		}
		else
		{
			__CubeeApp_sendStatus(this, CUBEE_APP_RCV_CMD_DEACTIVATE);
		}
	}

	memset(&wanDataPacket, 0, sizeof(WAN_Ctrl_Data_Packet_t));
	wanDataPacket.dataType = WAN_CTRL_RCV_DATA_COMMAND;
	if(WAN_Ctrl_rcvData(this->wanControler, &wanDataPacket) != CUBEE_ERROR_OK){
		/* We have finished accessing the shared resource.  Release the	 semaphore. */
		xSemaphoreGive(wanSemaphore);
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Process CUBEE JSON */
	wanJson = cJSON_Parse((const char *) wanDataPacket.dataBuffer);
	if(wanJson == NULL)
	{
		ESP_LOGE(LOG_HEADER,"Json over WAn Network is empty or invalid.");
		/* We have finished accessing the shared resource.  Release the	 semaphore. */
		xSemaphoreGive(wanSemaphore);
		return CUBEE_ERROR_UNDEFINED;
	}
#ifdef DEBUG_CUBEE_APP
	ESP_LOGI(LOG_HEADER,"WAN Json received: %s", cJSON_PrintUnformatted(wanJson));
#endif /*DEBUG_CUBEE_APP*/
	if(cJSON_GetObjectItem(wanJson, "appCommand") == NULL)
	{
		ESP_LOGE(LOG_HEADER,"Json over WAn Network is invalid, missing appCommand.");
		/* We have finished accessing the shared resource.  Release the	 semaphore. */
		xSemaphoreGive(wanSemaphore);
		return CUBEE_ERROR_UNDEFINED;
	}
	else
	{
		appCommand = cJSON_GetObjectItem(wanJson, "appCommand")->valueint;
	}

	if((appCommand == CUBEE_APP_RCV_CMD_DEACTIVATE) && (this->cubeeState == true))
	{
		ESP_LOGI(LOG_HEADER,"WAN command Received, DEACTIVATE");
		if (IOMgr_deactivate(this->ioMgr) == CUBEE_ERROR_OK)
		{
			ESP_LOGI(LOG_HEADER,"CUBEE successfully deactivated");
			this->cubeeState = false;
			this->sendState = true;
		}
	}
	else if((appCommand == CUBEE_APP_RCV_CMD_ACTIVATE) && (this->cubeeState == false))
	{
		ESP_LOGI(LOG_HEADER,"WAN command Received, ACTIVATE");
		if (IOMgr_activate(this->ioMgr) == CUBEE_ERROR_OK)
		{
			ESP_LOGI(LOG_HEADER,"CUBEE successfully activated");
			this->cubeeState = true;
			this->sendState = true;
		}
	}
	else if(appCommand == CUBEE_APP_RCV_CMD_LED)
	{
		ESP_LOGI(LOG_HEADER,"WAN command Received, LED");
		if(IOMgr_toogleLED(this->ioMgr) == CUBEE_ERROR_OK)
		{
			ESP_LOGI(LOG_HEADER,"LED command successfully executed");
		}
	}
#ifdef USING_DB9
	else if(appCommand == CUBEE_APP_RCV_CMD_DB9_RULE)
	{
		ESP_LOGI(LOG_HEADER,"WAN command Received, DB9_RULE");
		db9RUleJson = cJSON_GetObjectItem(wanJson,"db9rule");
		if(db9RUleJson == NULL)
		{
			ESP_LOGE(LOG_HEADER,"Json over WAn Network is invalid, missing db9rule.");
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(wanSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}
		else if(__CubeeApp_parseDB9Rule(this, &db9RuleBuffer, db9RUleJson) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error parsing DB9 rule JSON");
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(wanSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}
		else
		{
			/* Send rule to DB9 app */
			DB9App_sendDB9Rule(this->db9App,&db9RuleBuffer);
		}
	}
	else if(appCommand == CUBEE_APP_RCV_CMD_DB9)
	{
		ESP_LOGI(LOG_HEADER,"WAN command Received, DB9");
		if(cJSON_GetObjectItem(wanJson, "db9Command") == NULL)
		{
			ESP_LOGE(LOG_HEADER,"Json over WAn Network is invalid, missing db9Command.");
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(wanSemaphore);
			return CUBEE_ERROR_UNDEFINED;
		}
		else
		{
			this->db9App->db9Command.value = cJSON_GetObjectItem(wanJson, "db9Command")->valueint;
			/* Send command to DB9 app */
			DB9App_sendSignal(this->db9App,DB9_APP_SIGNAL_COMMAND);
		}

	}
#endif /* USING_DB9 */

	/* If required, send CUBEE state to PAN and WAn network */
	if(this->sendState)
	{
		if(this->cubeeState == true)
		{
			__CubeeApp_sendStatus(this, CUBEE_APP_RCV_CMD_ACTIVATE);
		}
		else
		{
			__CubeeApp_sendStatus(this, CUBEE_APP_RCV_CMD_DEACTIVATE);
		}
	}

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(wanSemaphore);

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __CubeeApp_STM_Action_sendButtonCmd(CubeeApp_t* this)
{
	CubeeErrorCode_t error;

	/* Send authentication signal to PAN network */
	PANController_sendAuthenticationCommand(this->panControler);

	/* Send button command to PAN and WAN network */
	error = __CubeeApp_sendCommand(this,CUBEE_APP_SND_CMD_BUTTON);
	if(error != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Button command sending failed ");
		return error;
	}



	ESP_LOGI(LOG_HEADER,"Button command sent");

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __CubeeApp_STM_Action_sendInstantAlarms(CubeeApp_t* this)
{
	cJSON * alarmJSonArray;
	cJSON * alarmJSon;
	cJSON * alarmJSonObjects;
	uint16_t alarmCount = 0;
	StorageMgr_AlarmUnion_t alarmUnion;
	char  * alarmJsonString;

	/* read Cubee configuration */
	memset(&cubeeCfgBuffer, 0, sizeof(CubeeConfig_t));
	if(StorageMgr_readCubeeCfg(this->storageMgr,&cubeeCfgBuffer) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Send alarm failed, it wasn't possible read the current CUBEE configuration");
		return CUBEE_ERROR_UNDEFINED;
	}

	alarmJSon = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(alarmJSon, "idCubee", cJSON_CreateString((const char*)cubeeCfgBuffer.idCubee));
	alarmJSonArray = cJSON_CreateArray();
	cJSON_AddItemToObjectCS(alarmJSon,"alarms",alarmJSonArray);
	alarmJSonObjects = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(alarmJSonObjects,"timeStamp",cJSON_CreateNumber(0));
	cJSON_AddItemToObjectCS(alarmJSonObjects,"code",cJSON_CreateNumber(0));
	cJSON_AddItemToObjectCS(alarmJSonObjects,"value",cJSON_CreateNumber(0));
	while(xQueueReceive(instantAlarmsQueue, &alarmUnion, INSTANT_ALARM_QUEUE_TIMEOUT) == pdTRUE)
	{
		alarmCount++;
		cJSON_ReplaceItemInObject(alarmJSonObjects,"timeStamp",cJSON_CreateNumber(alarmUnion.alarm.timeStamp));
		cJSON_ReplaceItemInObject(alarmJSonObjects,"code",cJSON_CreateNumber(alarmUnion.alarm.code));
		cJSON_ReplaceItemInObject(alarmJSonObjects,"value",cJSON_CreateNumber(alarmUnion.alarm.value));
		cJSON_AddItemToArray(alarmJSonArray,alarmJSonObjects);
	}
	if(alarmCount > 0)
	{
		/* Send alarm JSON */
		memset(&wanDataPacket, 0, sizeof(WAN_Ctrl_Data_Packet_t));
		alarmJsonString = cJSON_PrintUnformatted(alarmJSon);
		memmove(wanDataPacket.dataBuffer, (uint8_t *) alarmJsonString, strlen(alarmJsonString));
		wanDataPacket.dataType = WAN_CTRL_SND_DATA_ALARM;
		wanDataPacket.dataBufferSize = strlen(alarmJsonString);
		if(WAN_Ctrl_sendData(this->wanControler, &wanDataPacket, true) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error sending instant alarms to the server, value: %s", alarmJsonString);
			return CUBEE_ERROR_UNDEFINED;
		}
		else
		{
			ESP_LOGI(LOG_HEADER,"Instant alarms successfully sent to the server value: %s", alarmJsonString);
		}
	}

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __CubeeApp_STM_Action_sendStoredAlarms(CubeeApp_t* this)
{
	cJSON * alarmJSonArray;
	cJSON * alarmJSon;
	cJSON * alarmJSonObjects;
	uint16_t alarmCount = 0;
	uint16_t alarmIndex = 0;
	StorageMgr_AlarmUnion_t * alarmPointer;
	char  * alarmJsonString;

	if(StorageMgr_isAlarmMemoryEmpty(this->storageMgr))
	{
		return CUBEE_ERROR_OK;
	}

	/* read Cubee configuration */
	memset(&cubeeCfgBuffer, 0, sizeof(CubeeConfig_t));
	if(StorageMgr_readCubeeCfg(this->storageMgr,&cubeeCfgBuffer) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Send alarm failed, it wasn't possible read the current CUBEE configuration");
		return CUBEE_ERROR_UNDEFINED;
	}

	alarmJSon = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(alarmJSon, "idCubee", cJSON_CreateString((const char*)cubeeCfgBuffer.idCubee));

	memset(&wanDataPacket, 0, sizeof(WAN_Ctrl_Data_Packet_t));
	this->storageMgr->alarmDataBuffer = wanDataPacket.dataBuffer;
	this->storageMgr->alarmDataBufferSize = WAN_CTRL_MAX_DATA_SIZE / 4; /* When passing to JSon format the length of alarm data will increase about 4 times */
	if(StorageMgr_readAlarms(this->storageMgr, &alarmCount) == CUBEE_ERROR_OK)
	{
		if(alarmCount == 0)
		{
			ESP_LOGE(LOG_HEADER, "Send alarm failed, no valid alarms read from flash memory");
			return CUBEE_ERROR_UNDEFINED;
		}
		alarmJSonArray = cJSON_CreateArray();
		alarmPointer = (StorageMgr_AlarmUnion_t *)  wanDataPacket.dataBuffer;

		/* Build alarm JSon */
		cJSON_AddItemToObjectCS(alarmJSon,"alarms",alarmJSonArray);
		for(alarmIndex = 0; alarmIndex < alarmCount; alarmIndex++)
		{
			alarmJSonObjects = cJSON_CreateObject();
			cJSON_AddItemToObjectCS(alarmJSonObjects,"timeStamp",cJSON_CreateNumber(alarmPointer[alarmIndex].alarm.timeStamp));
			cJSON_AddItemToObjectCS(alarmJSonObjects,"code",cJSON_CreateNumber(alarmPointer[alarmIndex].alarm.code));
			cJSON_AddItemToObjectCS(alarmJSonObjects,"value",cJSON_CreateNumber(alarmPointer[alarmIndex].alarm.value));
			cJSON_AddItemToArray(alarmJSonArray,alarmJSonObjects);
		}

		memset(&wanDataPacket, 0, sizeof(WAN_Ctrl_Data_Packet_t));
		alarmJsonString = cJSON_PrintUnformatted(alarmJSon);
		memmove(wanDataPacket.dataBuffer, (uint8_t *) alarmJsonString, strlen(alarmJsonString));
		wanDataPacket.dataType = WAN_CTRL_SND_DATA_ALARM;
		wanDataPacket.dataBufferSize = strlen(alarmJsonString);


		if(WAN_Ctrl_sendData(this->wanControler, &wanDataPacket, true) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error sending alarms to the server, value: %s", alarmJsonString);
			return CUBEE_ERROR_UNDEFINED;
		}

		ESP_LOGI(LOG_HEADER,"Alarms successfully sent to the server value: %s", alarmJsonString);
		StorageMgr_confirmReadAlarms(this->storageMgr);
	}
	else
	{
		ESP_LOGE(LOG_HEADER, "Send alarm failed, it wasn't possible read the alarms stored in flash memory");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

#ifdef USING_SENSOR_MGR
static CubeeErrorCode_t __CubeeApp_STM_Action_sendMeasurement(CubeeApp_t* this)
{
	cJSON* measurementJSon;
	int32_t currentSample = 0;
	int32_t average = 0;
	uint16_t samplesCount = 0;
	char  * measurementJsonString;

	if((StorageMgr_isSampleMemoryEmpty(this->storageMgr)) && SensorMgr_isEmpty(this->sensorMgr,this->sensorId))
	{
		ESP_LOGI(LOG_HEADER,"The Cubee hasn't measurements to be sent");
		return CUBEE_ERROR_OK;
	}
	else if(!StorageMgr_isSampleMemoryEmpty(this->storageMgr))
	{
		/* Calculates the avarage of stored samples*/
		memset(sampleBuffer.sampleArray, 0, sizeof(StorageMgr_SampleUnion_t));
		this->storageMgr->sampleDataBuffer = (uint8_t *)&sampleBuffer;
		this->storageMgr->sampleDataBufferSize = sizeof(StorageMgr_SampleUnion_t); /* When passing to JSon format the length of alarm data will increase about 4 times */
		if((StorageMgr_readSamples(this->storageMgr, &samplesCount) == CUBEE_ERROR_OK) && (samplesCount != 0))
		{
			average = sampleBuffer.sample.value;
			/* Erase stored samples */
			StorageMgr_confirmReadSamples(this->storageMgr);
		}
	}

	/* Read current samples */
#ifdef USING_CURRENT_SENSOR
	if(SensorMgr_readAvrgValue(this->sensorMgr,this->sensorId,&currentSample) == CUBEE_ERROR_OK)
	{
		average = (average + currentSample) / 2;
	}
#endif
#ifdef USING_ACCELEROMETER
	if(SensorMgr_readSTDEV(this->sensorMgr,this->sensorId,&currentSample) == CUBEE_ERROR_OK)
	{
		average = (average + currentSample) / 2;
	}
#endif

	/* Reset the samples */
	SensorMgr_resetSensor(this->sensorMgr,this->sensorId);
	if( xSemaphoreTake( wanSemaphore, WAN_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking the WAN semaphore");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* read Cubee configuration */
	StorageMgr_readCubeeCfg(this->storageMgr,&cubeeCfgBuffer);

	/* Build and send measurement JSon */
	measurementJSon = cJSON_CreateObject();
	cJSON_AddItemToObjectCS(measurementJSon, "idCubee", cJSON_CreateString((const char*)cubeeCfgBuffer.idCubee));
	cJSON_AddNumberToObject(measurementJSon,"measurement",average);

	memset(&wanDataPacket, 0, sizeof(WAN_Ctrl_Data_Packet_t));
	measurementJsonString = cJSON_PrintUnformatted(measurementJSon);
	memmove(wanDataPacket.dataBuffer, (uint8_t *) measurementJsonString, strlen(measurementJsonString));
	wanDataPacket.dataType = WAN_CTRL_SND_DATA_MEASUREMENT;
	wanDataPacket.dataBufferSize = strlen(measurementJsonString);

	if(WAN_Ctrl_sendData(this->wanControler, &wanDataPacket, true) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error sending measurement to server, value: %s", measurementJsonString);
		/* Store saple to be sent later */
		memset(sampleBuffer.sampleArray, 0, sizeof(StorageMgr_SampleUnion_t));
		sampleBuffer.sample.timeStamp = 0;
		sampleBuffer.sample.value = average;

		if(StorageMgr_saveSample(this->storageMgr, &sampleBuffer) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error storing updated measurement average");
		}
		else
		{
			ESP_LOGI(LOG_HEADER,"The measurement average was stored: %d", sampleBuffer.sample.value);
		}

		xSemaphoreGive(wanSemaphore);
		return CUBEE_ERROR_UNDEFINED;
	}

	xSemaphoreGive(wanSemaphore);
	ESP_LOGI(LOG_HEADER,"Measurement successfully sent to server, value: %s", measurementJsonString);

	return CUBEE_ERROR_OK;
}
#endif








