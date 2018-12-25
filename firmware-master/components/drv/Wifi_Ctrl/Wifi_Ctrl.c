
/* Module Dependencies */
#include "Wifi_Ctrl.h"
#include "Wifi_Ctrl_Cfg.h"
#include "../../../main/ProjectCfg.h"

/* FreeRTOS */
#include "freertos/event_groups.h"

/* ESP Libraries */
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "tcpip_adapter.h"
#include "esp_wifi.h"


/* LWIP - TCP/IP Protocol Implementation */
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#define LOG_HEADER 						"[WIFI DRV] "
#define CONNECTED_BIT					(1<<1)				/*!< BIT1 */


#define xDEBUG_WIFI_CTRL

/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

/**
 * @brief Task code to periodically stimulate the wifi controller state machine with the signals received in the signal queue.
 *
 * @param pvParameters : receives the wifi controller instance containing the state machine stimulated by the task
*/
static void __Wifi_Ctrl_taskCode( void * pvParameters );

/**
 * @brief Function that implements the wifi controller state machine.
 *
 * @param this - pointer to wifi controller component instance.
 * @param msg - Signal used to stimulate the state machine.
*/
static bool __Wifi_Ctrl_STMStimulate(Wifi_Ctrl_t * this, Wifi_Ctrl_Signals_t msg);


static esp_err_t __Wifi_Ctrl_SytemEventHandler(void *ctx, system_event_t *event);

/************************************************************************************************/
/* State Machine Conditions  Definition */
/************************************************************************************************/

/**
 * @brief This function informs if the wifi is enabled
 *
 * @param this - pointer to wifi controller component instance.
 *
 * @return
 * @arg true, if the wifi is enabled.
 * @arg false, if the wifi is disabled, or an invalid parameter was passed.
*/
static bool __Wifi_Ctrl_STM_Condition_wifiEnabled(Wifi_Ctrl_t * this);

/**
 * @brief This function evaluates if the connection timeout was reached after the CONNECTING state begins.
 *
 * @param this - pointer to wifi controller component instance.
 *
 * @arg true, if the connection time is bigger than configured connection timeout.
 * @arg false, if the connection time is less than configured connection timeout, or an invalid parameter was passed.
*/
static bool __Wifi_Ctrl_STM_Condition_connectionTimeout(Wifi_Ctrl_t * this);

/**
 * @brief This function evaluates if the sleep timeout was reached after the SLEEP state begins.
 *
 * @param this - pointer to wifi controller component instance.
 *
 * @arg true, if the sleeping time is bigger than configured sleeping timeout.
 * @arg false, if the sleeping time is less than configured sleeping timeout, or an invalid parameter was passed.
*/
static bool __Wifi_Ctrl_STM_Condition_sleepTimeout(Wifi_Ctrl_t * this);

/************************************************************************************************/
/* State Machine Actions  Definition */
/************************************************************************************************/

/**
 * @brief This function initializes the wifi controller state machine on INITIALIZATION state.
 *
 * @param this - pointer to wifi controller component instance.
 *
 * @arg CUBEE_ERROR_OK, if the state machine was successfully initialized.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_init(Wifi_Ctrl_t * this);

/**
 * @brief This function sets wifi parameters.
 *
 * @param this - pointer to wifi controller component instance.
 *
 * @arg CUBEE_ERROR_OK, if the configuration was successfully executed.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_config(Wifi_Ctrl_t * this);

/**
 * @brief  This function calls the ESP WIFI API to start the WIFI Station with the current configuration.
 *
 * @param this - pointer to wifi controller component instance.
 *
 * @arg CUBEE_ERROR_OK, if the start wifi command was successfully executed.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_startWifi(Wifi_Ctrl_t * this);

/**
 * @brief  This function calls the BLE GAP API to stop the advertising.
 *
 * @arg CUBEE_ERROR_OK, if the stop advertising command was successfully executed.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_stopWifi(Wifi_Ctrl_t * this);

/**
 * @brief  This function calls the ESP WIFI API to connect wifi Station with the configured wifi access point.
 *
 * @param this - pointer to wifi contoller component instance.
 *
 * @arg CUBEE_ERROR_OK, if the connection command was successfully executed.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_connectWifi(Wifi_Ctrl_t * this);

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t eventGroup;

static wifi_config_t wifiConfig;

static Wifi_Ctrl_t wifiCtrlInstance =
{
		.initialized = false,
		.wifiEnable = true,
};


/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t Wifi_Ctrl_init(Wifi_Ctrl_t * this)
{
	CubeeErrorCode_t cubeeError;
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;
	wifi_init_config_t wifiInitCfg = WIFI_INIT_CONFIG_DEFAULT();


	if(this == NULL)
	{
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Verify if the Wifi Controller is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing Wifi Controller... ");

	this->wifiControllerCfg = Wifi_Ctrl_Cfg_getInstance();
	cubeeError = Wifi_Ctrl_Cfg_init(this->wifiControllerCfg);
    if (cubeeError != CUBEE_ERROR_OK) {
        ESP_LOGE(LOG_HEADER, "Wifi config initialization failed");
        return CUBEE_ERROR_UNDEFINED;
    }

    /* Initializes TCP/IP stack */
    tcpip_adapter_init();

    /* Creating event group to signal when we are connected*/
    eventGroup = xEventGroupCreate();

    /* Initialize the event loop, create the event handler and task to manage wifi events */
    if(esp_event_loop_init(__Wifi_Ctrl_SytemEventHandler, NULL) != ESP_OK)
    {
    	ESP_LOGE(LOG_HEADER, "TCP/IP adapter initialization failed");
    	return CUBEE_ERROR_UNDEFINED;
    }

    /*Alloc resource for WiFi driver and start WiFi task*/
    if(esp_wifi_init(&wifiInitCfg) != ESP_OK)
    {
    	ESP_LOGE(LOG_HEADER, "Wifi stack initialization failed");
    	return CUBEE_ERROR_UNDEFINED;
    }

    /* Create state machine signals queue */
    this->signalQueueHandle = xQueueCreate(WIFI_CTRL_SIGNAL_QUEUE_LENGTH, WIFI_CTRL_SIGNAL_QUEUE_ITEM_SIZE);
	this->semaphore = xSemaphoreCreateMutex();
	if(this->semaphore == NULL)
	{
	  	ESP_LOGE(LOG_HEADER, "Error creating wifi controller semaphore");
	   	return CUBEE_ERROR_UNDEFINED;
	}

     /* Call init function of stm  for instance  */
     __Wifi_Ctrl_STM_Action_init(this);

     /* Create the task, storing the handle. */
     xReturned = xTaskCreatePinnedToCore(
     					__Wifi_Ctrl_taskCode,      	/* Function that implements the task. */
 						"Wifi_Ctrl_task", 			/* Text name for the task. */
 						2048,      					/* Stack size in words, not bytes. */
 						( void * ) this,    		/* Parameter passed into the task. */
 						WIFI_CTRL_TASK_PRIORITY,	/* Priority at which the task is created. */
 						&xHandle,					/* Used to pass out the created task's handle. */
						WIFI_CTRL_TASK_CORE);     	/* Specifies the core to run the task */
 	if( xReturned != pdPASS )
     {
         return CUBEE_ERROR_UNDEFINED;
     }

    this->initialized = true;

    ESP_LOGI(LOG_HEADER,"Wifi Controller initialized ");

    return CUBEE_ERROR_OK;
}



Wifi_Ctrl_t * Wifi_Ctrl_getInstance(void)
{
	return &wifiCtrlInstance;
}

Wifi_Ctrl_Cfg_t * Wifi_Ctrl_getCfg(Wifi_Ctrl_t* this)
{
	if(this == NULL)
	{
		return NULL;
	}

	return this->wifiControllerCfg;
}

bool Wifi_Ctrl_isInitialized(Wifi_Ctrl_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

CubeeErrorCode_t Wifi_Ctrl_isConnected(Wifi_Ctrl_t * this)
{
	uint32_t eventGroupBits = 0;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER getting connection status ");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	eventGroupBits = (uint32_t) xEventGroupGetBits(eventGroup);
#ifdef DEBUG_WIFI_CTRL
	ESP_LOGI(LOG_HEADER, "Connection status, event group bits: %#02x", eventGroupBits);
#endif	/* DEBUG_WIFI_CTRL */

	/*Test connection bit*/
	if((eventGroupBits & CONNECTED_BIT) == CONNECTED_BIT)
	{
		return CUBEE_ERROR_OK;
	}

	return CUBEE_ERROR_UNDEFINED;
}

CubeeErrorCode_t Wifi_Ctrl_waitConnection(Wifi_Ctrl_t * this, uint32_t timeout)
{
	uint32_t eventGroupBits = 0;
	uint32_t timoutTicks;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER waiting connection ");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(timeout == 0)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER waiting connection ");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	timoutTicks = timeout / portTICK_PERIOD_MS;

	/* Wait for the callback to set the CONNECTED_BIT in the event group. */
	eventGroupBits = xEventGroupWaitBits(eventGroup, CONNECTED_BIT, false, true, timoutTicks);
	if( ( eventGroupBits & CONNECTED_BIT ) == CONNECTED_BIT )
	{
		/* The wifi is connected */
#ifdef	DEBUG_WIFI_CTRL
		ESP_LOGI(LOG_HEADER, "Valid wifi connection");
#endif /* DEBUG_WIFI_CTRL */
		return CUBEE_ERROR_OK;
	}

	return CUBEE_ERROR_UNDEFINED;
}

CubeeErrorCode_t Wifi_Ctrl_updateCfg(Wifi_Ctrl_t * this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER updating wifi configuration ");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	ESP_LOGI(LOG_HEADER, "Updating WIFI configuration...");
	ESP_LOGI(LOG_HEADER, "Setting Access Point, previous password: %s, previous ssid: %s", (char *) wifiConfig.sta.password, (char *) wifiConfig.sta.ssid);

	/* Clean ssid and password */
	memset((char *)wifiConfig.sta.ssid, 0, SSID_MAX_SIZE);
	memset((char *) wifiConfig.sta.password, 0, PASSWORD_MAX_SIZE);

	/* Updating wifi config*/
	memmove(wifiConfig.sta.ssid,this->wifiControllerCfg->wifiSSID,SSID_MAX_SIZE);
	memmove(wifiConfig.sta.password,this->wifiControllerCfg->wifiPassword,PASSWORD_MAX_SIZE);

    /*  Update configuration of the ESP32 station */
    if(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifiConfig) != ESP_OK)
    {
    	ESP_LOGE(LOG_HEADER, "Wifi config update failed ");
    	return CUBEE_ERROR_UNDEFINED;
    }

   	/* The wifi controller was successfully configured, send the WIFI_CTRL_SIGNAL_CONFIGURED signal to the state machine signal queue*/
    Wifi_Ctrl_sendSignal(this, WIFI_CTRL_SIGNAL_CONFIGURED);

    ESP_LOGI(LOG_HEADER, "Setting Access Point, updated password: %s, updated ssid: %s", (char *) wifiConfig.sta.password, (char *) wifiConfig.sta.ssid);
    ESP_LOGI(LOG_HEADER, "Wifi configuration successfully updated" )


	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t Wifi_Ctrl_sendSignal(Wifi_Ctrl_t * this, Wifi_Ctrl_Signals_t rcvSignal)
{
	Wifi_Ctrl_Signals_t signal = WIFI_CTRL_NOSIG;

	if((this == NULL) || (rcvSignal >= WIFI_CTRL_SIGNAL_MAX))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER sending signal to Wifi state machine");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->semaphore, WIFI_CTRL_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking wifi controller semaphore");
		return CUBEE_ERROR_TIMEOUT;
	}

	switch(rcvSignal)
	{
	case WIFI_CTRL_SIGNAL_INITIALIZED:
		if(this->stm.mainState.activeSubState == WIFI_CTRL_STATE_INITIALIZATION)
		{
	    	signal = WIFI_CTRL_SIGNAL_INITIALIZED;
		}
		break;

	case WIFI_CTRL_SIGNAL_CONFIGURED:
		if((this->stm.mainState.activeSubState == WIFI_CTRL_STATE_CONFIGURATION) ||
				(this->stm.mainState.activeSubState == WIFI_CTRL_STATE_SLEEPING))
		{
	    	signal = WIFI_CTRL_SIGNAL_CONFIGURED;
		}
		break;

	case WIFI_CTRL_SIGNAL_CONNECTED:
		if(this->stm.mainState.activeSubState == WIFI_CTRL_STATE_CONNECTING)
		{
	    	signal = WIFI_CTRL_SIGNAL_CONNECTED;
		}
		break;

	case WIFI_CTRL_SIGNAL_DISCONNECTED:
		if(this->stm.mainState.activeSubState == WIFI_CTRL_STATE_CONNECTED)
		{
	    	signal = WIFI_CTRL_SIGNAL_DISCONNECTED;
		}
		break;

	case WIFI_CTRL_SIGNAL_STARTED:
		if((this->stm.mainState.activeSubState == WIFI_CTRL_STATE_CONFIGURATION) ||
				(this->stm.mainState.activeSubState == WIFI_CTRL_STATE_SLEEPING))
		{
	    	signal = WIFI_CTRL_SIGNAL_STARTED;
		}
		break;

	case WIFI_CTRL_SIGNAL_STOPPED:
		if((this->stm.mainState.activeSubState == WIFI_CTRL_STATE_CONNECTING) ||
				(this->stm.mainState.activeSubState == WIFI_CTRL_STATE_CONNECTED))
		{
	    	signal = WIFI_CTRL_SIGNAL_STOPPED;
		}
		break;

	default:
		break;
	}

	if(signal != WIFI_CTRL_NOSIG)
	{
		/* Send the wifi controller signal to the state machine signal queue*/
		if(xQueueGenericSend(this->signalQueueHandle, &signal, WIFI_CTRL_SIGNAL_QUEUE_TIMEOUT, queueSEND_TO_BACK) != pdTRUE)
		{
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(this->semaphore);
			ESP_LOGE(LOG_HEADER,"Error sending signal to wifi state machine, signal queue is full");
			return CUBEE_ERROR_UNDEFINED;
		}
	}

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->semaphore);

	return CUBEE_ERROR_OK;
}


/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

static void __Wifi_Ctrl_taskCode( void * pvParameters )
{
	Wifi_Ctrl_t * wifiCtrl = (Wifi_Ctrl_t *) pvParameters;
	Wifi_Ctrl_Signals_t rcvSignal;

    for( ;; )
    {

    	/* Get signal from queue */
    	if(xQueueReceive(wifiCtrl->signalQueueHandle,&rcvSignal,WIFI_CTRL_SIGNAL_QUEUE_TIMEOUT) == pdTRUE)
    	{
    		/* Stimulates the state machine with signal received */
    		__Wifi_Ctrl_STMStimulate(wifiCtrl, rcvSignal);
    	}
    	else
    	{
    		__Wifi_Ctrl_STMStimulate(wifiCtrl, WIFI_CTRL_NOSIG);
    	}

		/* Block for 1 second. */
		 vTaskDelay( WIFI_CTRL_TASK_DELAY );
    }
}

static bool __Wifi_Ctrl_STMStimulate(Wifi_Ctrl_t * this, Wifi_Ctrl_Signals_t msg)
{
    bool evConsumed = false;

    switch (this->stm.mainState.activeSubState)
    {
    case WIFI_CTRL_STATE_INITIALIZATION:
    	/* Verify transition conditions */
    	if(msg == WIFI_CTRL_SIGNAL_INITIALIZED)
    	{
    		evConsumed = true;

    		/* transition actions */
    		__Wifi_Ctrl_STM_Action_config(this);

    		/* transition */
    		this->stm.mainState.activeSubState = WIFI_CTRL_STATE_CONFIGURATION;
    		ESP_LOGI(LOG_HEADER,"Transition, WIFI_CTRL_STATE_INITIALIZATION -> WIFI_CTRL_STATE_CONFIGURATION, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}
    	break;

    case WIFI_CTRL_STATE_CONFIGURATION:
    	/* Verify transition conditions */
    	if(msg == WIFI_CTRL_SIGNAL_STARTED)
    	{
    		evConsumed = true;

    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = WIFI_CTRL_STATE_CONNECTING;
    		FSM_startTime(&(this->stm.Connecting));
    		ESP_LOGI(LOG_HEADER,"Transition, WIFI_CTRL_STATE_CONFIGURATION -> WIFI_CTRL_STATE_CONNECTING");
    	}
    	else if((msg == WIFI_CTRL_SIGNAL_CONFIGURED) && __Wifi_Ctrl_STM_Condition_wifiEnabled(this))
    	{
    		evConsumed = true;

    		/* transition actions */
    		__Wifi_Ctrl_STM_Action_startWifi(this);

    		/* transition */
    	}
    	break;

    case WIFI_CTRL_STATE_CONNECTING:
    	/* Verify transition conditions */
    	if(msg == WIFI_CTRL_SIGNAL_STOPPED)
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = WIFI_CTRL_STATE_SLEEPING;
    		FSM_startTime(&(this->stm.Sleeping));
    		ESP_LOGI(LOG_HEADER,"Transition, WIFI_CTRL_STATE_CONNECTING -> WIFI_CTRL_STATE_SLEEPING");
    	}
    	else if(msg == WIFI_CTRL_SIGNAL_CONNECTED)
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = WIFI_CTRL_STATE_CONNECTED;
    		ESP_LOGI(LOG_HEADER,"Transition, WIFI_CTRL_STATE_CONNECTING -> WIFI_CTRL_STATE_CONNECTED");
    	}
    	else if(__Wifi_Ctrl_STM_Condition_connectionTimeout(this))
    	{
    		__Wifi_Ctrl_STM_Action_stopWifi(this);
    	}
    	else
    	{
    		__Wifi_Ctrl_STM_Action_connectWifi(this);
    		/* Block for 5s. */
    		 vTaskDelay( 1000 / portTICK_PERIOD_MS );
    	}

        break;

    case WIFI_CTRL_STATE_CONNECTED:
    	/* Verify transition conditions */
    	if(msg == WIFI_CTRL_SIGNAL_STOPPED)
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = WIFI_CTRL_STATE_SLEEPING;
    		FSM_startTime(&(this->stm.Sleeping));
    		ESP_LOGI(LOG_HEADER,"Transition, WIFI_CTRL_STATE_CONNECTED -> WIFI_CTRL_STATE_SLEEPING");
    	}
    	else if(msg == WIFI_CTRL_SIGNAL_DISCONNECTED)
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = WIFI_CTRL_STATE_CONNECTING;
    		FSM_startTime(&(this->stm.Connecting));
    		ESP_LOGI(LOG_HEADER,"Transition, WIFI_CTRL_STATE_CONNECTED -> WIFI_CTRL_STATE_CONNECTING");
    	}

        break;

    case WIFI_CTRL_STATE_SLEEPING:
    	/* Verify transition conditions */
    	if(msg == WIFI_CTRL_SIGNAL_STARTED)
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = WIFI_CTRL_STATE_CONNECTING;
    		FSM_startTime(&(this->stm.Connecting));
    		ESP_LOGI(LOG_HEADER,"Transition, WIFI_CTRL_STATE_SLEEPING -> WIFI_CTRL_STATE_CONNECTING");
    	}
    	else if((msg == WIFI_CTRL_SIGNAL_CONFIGURED) && __Wifi_Ctrl_STM_Condition_wifiEnabled(this))
    	{
    		evConsumed = true;

    		/* transition actions */
    		__Wifi_Ctrl_STM_Action_startWifi(this);

    		/* transition */
    	}
    	else if(__Wifi_Ctrl_STM_Condition_sleepTimeout(this) && __Wifi_Ctrl_STM_Condition_wifiEnabled(this))
    	{
    		evConsumed = true;
    		/* transition actions */
    		__Wifi_Ctrl_STM_Action_startWifi(this);

    		/* transition */
    	}

        break;

    default:
        break;
    }

    return evConsumed;
}


static esp_err_t __Wifi_Ctrl_SytemEventHandler(void *ctx, system_event_t *event)
{
#ifdef	DEBUG_WIFI_CTRL
	ESP_LOGI(LOG_HEADER, "Wifi Controller, Event id: %d, task priority: %d, task core: %d", event->event_id, uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_WIFI_CTRL */

    switch(event->event_id) {
    case SYSTEM_EVENT_WIFI_READY:
    	ESP_LOGI(LOG_HEADER, "SYSTEM_EVENT_WIFI_READY, ESP32 WiFi ready");

    	break;

    case SYSTEM_EVENT_STA_START:
    	ESP_LOGI(LOG_HEADER, "SYSTEM_EVENT_STA_START, ESP32 station start");
       	/* The wifi was successfully started, send the WIFI_CTRL_SIGNAL_STARTED signal to the state machine signal queue*/
        Wifi_Ctrl_sendSignal(&wifiCtrlInstance, WIFI_CTRL_SIGNAL_STARTED);
        break;

    case SYSTEM_EVENT_STA_STOP:
        	ESP_LOGI(LOG_HEADER, "SYSTEM_EVENT_STA_STOP, ESP32 station was stopped");
           	/* The wifi was successfully stopped, send the WIFI_CTRL_SIGNAL_STOPPED signal to the state machine signal queue*/
            Wifi_Ctrl_sendSignal(&wifiCtrlInstance, WIFI_CTRL_SIGNAL_STOPPED);
            break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
    	ESP_LOGI(LOG_HEADER, "SYSTEM_EVENT_STA_DISCONNECTED, ESP32 station disconnected, reason: %d ", event->event_info.disconnected.reason);
    	xEventGroupClearBits(eventGroup, CONNECTED_BIT);
      	/* The wifi was successfully disconnected, send the WIFI_CTRL_SIGNAL_DISCONNECTED signal to the state machine signal queue*/
        Wifi_Ctrl_sendSignal(&wifiCtrlInstance, WIFI_CTRL_SIGNAL_DISCONNECTED);
        break;

    case SYSTEM_EVENT_STA_CONNECTED:
    	ESP_LOGI(LOG_HEADER, "SYSTEM_EVENT_STA_CONNECTED, ESP32 station connected to AP, SSID: %s ", event->event_info.connected.ssid);
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
    	ESP_LOGI(LOG_HEADER, "SYSTEM_EVENT_STA_GOT_IP, ESP32 station got IP from connected AP");
    	xEventGroupSetBits(eventGroup, CONNECTED_BIT);
      	/* The wifi was successfully connected, send the WIFI_CTRL_SIGNAL_CONNECTED signal to the state machine signal queue*/
        Wifi_Ctrl_sendSignal(&wifiCtrlInstance, WIFI_CTRL_SIGNAL_CONNECTED);
        break;



    default:
        break;
    }
    return ESP_OK;
}

/************************************************************************************************/
/* State Machine Conditions  Implementation */
/************************************************************************************************/


static bool __Wifi_Ctrl_STM_Condition_wifiEnabled(Wifi_Ctrl_t * this)
{
	return this->wifiEnable;
}


static bool __Wifi_Ctrl_STM_Condition_connectionTimeout(Wifi_Ctrl_t * this)
{
	uint64_t timeElapsed = FSM_getTimeElapsedasd(&(this->stm.Connecting), TIME_MILLISECONDS);

	return  (timeElapsed > WIFI_CTRL_CONNECTION_TIMEOUT_MS);
}


static bool __Wifi_Ctrl_STM_Condition_sleepTimeout(Wifi_Ctrl_t * this)
{
	uint64_t timeElapsed = FSM_getTimeElapsedasd(&(this->stm.Sleeping), TIME_MILLISECONDS);

	return  (timeElapsed > WIFI_CTRL_SLEEP_TIMEOUT_MS);
}

/************************************************************************************************/
/* State Machine Actions  Definition */
/************************************************************************************************/


static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_init(Wifi_Ctrl_t * this)
{

    /* State Initialization doesn't have substate */
    this->stm.Initialization.activeSubState = WIFI_CTRL_NOSTATE;

    /* State configuration doesn't have substate */
    this->stm.Configuration.activeSubState = WIFI_CTRL_NOSTATE;

	/* State connecting doesn't have substate*/
    this->stm.Connecting.activeSubState = WIFI_CTRL_NOSTATE;

    /* State connected doesn't have substate */
    this->stm.Connected.activeSubState = WIFI_CTRL_NOSTATE;

    /* State sleeping doesn't have substate */
    this->stm.Sleeping.activeSubState = WIFI_CTRL_NOSTATE;

    /* Initial State -> Initialization */
    this->stm.mainState.activeSubState = WIFI_CTRL_STATE_INITIALIZATION;
    FSM_startTime(&(this->stm.Initialization));

   	/* The wifi controller was successfully started, send the WIFI_CTRL_SIGNAL_INITIALIZED signal to the state machine signal queue*/
    Wifi_Ctrl_sendSignal(this, WIFI_CTRL_SIGNAL_INITIALIZED);



	return CUBEE_ERROR_OK;
}


static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_config(Wifi_Ctrl_t * this)
{
    /*Set the wifi configuration storage*/
    if(esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK)
    {
    	ESP_LOGE(LOG_HEADER, "Wifi storage definition failed ");
    	return CUBEE_ERROR_UNDEFINED;
    }

    /* Set the WiFi operating mode */
    if(esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK)
    {
    	ESP_LOGE(LOG_HEADER, "Wifi operation mode definition failed ");
    	return CUBEE_ERROR_UNDEFINED;
    }

	return CUBEE_ERROR_OK;
}


static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_startWifi(Wifi_Ctrl_t * this)
{
    if(esp_wifi_start() != ESP_OK)
    {
    	ESP_LOGE(LOG_HEADER, "Wifi start failed ");
    	return CUBEE_ERROR_UNDEFINED;
    }

	return CUBEE_ERROR_OK;
}


static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_stopWifi(Wifi_Ctrl_t * this)
{
	if(esp_wifi_stop() != ESP_OK)
	{
	  	ESP_LOGE(LOG_HEADER, "Wifi stop failed ");
	   	return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}


static CubeeErrorCode_t __Wifi_Ctrl_STM_Action_connectWifi(Wifi_Ctrl_t * this)
{
	if(esp_wifi_connect() != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "Wifi connection failed ");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}



