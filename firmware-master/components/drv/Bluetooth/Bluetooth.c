/**
 * @file Bluetooth.h
 * @version 1.1
 * @author Alex Fernandes
 * @date August 01, 2017
 **************************************************************************
 *
 * @brief  Driver Bluetooth
 * This module allows Bluetooth Low Energy (BLE) communication. It use the the Generic Access Profile (GAP) to broadcast advertising data
 * and connect to others BLE devices. Furthermore the driver has a GATT server that can be used to send/receive commands,
 * show the device status and receive configurations. The BLE driver implements a state machine to control the current state
 * and allows authentication on application level.
 *
 * @section References
 * @ref 1. BLUETOOTH SPECIFICATION Version 4.2
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   01-Jun-2017    Alex Fernandes
 * * Original version based on ESP-IDF security GATT server example
 *
 *  * Revision: 1.1   01-Aug-2017    Alex Fernandes
 * * Deep code refactoring and state machine implementation
 ******************************************************************/


/* Module Dependencies */
#include "Bluetooth.h"

/* External libraries */
#include "bt.h"
#include "esp_log.h"


#define LOG_HEADER 		"[BLUETOOTH DRV] "	/*!< Header used to print LOG messages */
#define xDEBUG_BLUETOOTH				 	/*!< Switch between  DEBUG_BLUETOOTH and xDEBUG_BLUETOOTH to activate or deactivate Debug messages, respectively */

/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

/**
 * @brief Callback function used to handle GAP (Generic Access Protocol) events coming from device bluetooth stack
 *
 * Sends the signals BLUETOOTH_SIGNAL_CONFIGURED, ADVERTISING_DATA_SET_EVT, BLUETOOTH_SIGNAL_ADVERTISNG_STARTED
 * and BLUETOOTH_SIGNAL_ADVERTISING_STOPPED to bluetooth state machine. Answers the security request generated
 * when connecting to a remote device.
 *
 * @param event : Event type
 * @param param : Pointer to event parameters
 */
static void __Bluetooth_gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

/**
 * @brief Callback function used to handle GATT Server (Generic Attribute Profile) events coming from device bluetooth stack
 *
 * Store the GATT interface associated with each profile registered, and forwards the GATT events to the specific GATT
 * interface handler function.
 *
 * @param event : Event type
 * @gatts_if : GATT interface type, different application on GATT client use different gatt_if
 * @param param : Pointer to event parameters
*/
static void __Bluetooth_gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/**
 * @brief Callback function used to handle GATT Server (Generic Attribute Profile) events coming from a specific GATT interface.
 *
 * Catches and handles all the GATT events from a specific GATT interface. Initializes the bluetooth server creating the Attribute Table
 * and starting the services used to BLE data exchange with remote devices. Receives command and configuration data from connected
 * devices that were already authenticated by the bluetooth driver. Sends the signals BLUETOOTH_SIGNAL_INITIALIZED,
 * BLUETOOTH_SIGNAL_DEVICE_CONNECTED and BLUETOOTH_SIGNAL_DEVICE_DISCONNECTED to bluetooth state machine.
 *
 * @param event : Event type
 * @gatts_if : GATT interface type, different application on GATT client use different gatt_if
 * @param param : Pointer to event parameters
 */
static void __Bluetooth_gattsProfileEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/**
 * @brief This function is used to receive a configuration coming from a connected and authenticated device.
 *
 * That function works only for configurations received in simple JSon format. It means that the first configuration character
 * needs to be '{' and the last one needs to be '}'. If the JSon has more than one '{' or '}' character, the configuration will not
 * be correctly received.
 *
 * In Bluetooth v4.0 and 4.1, the maximum size of the Data field is 27 bytes, so that functions needs to implement an
 * algorithm to receive the configuration packets and concatenate them.
 *
 * @param this - pointer to bluetooth component instance.
 * @param param : Pointer to event parameters containing the configuration packet received.
*/
static void __Bluetooth_gattsReceiveCfgHandler(Bluetooth_t * this, esp_ble_gatts_cb_param_t *param);

/**
 * @brief This function receives commands coming from connected and authenticated devices, and send the command to a
 * command queue.
 *
 * @param this - pointer to bluetooth component instance.
 * @param param : Pointer to event parameters containing the command packet received.
*/
static void __Bluetooth_gattsReceiveCmdHandler(Bluetooth_t * this, esp_ble_gatts_cb_param_t *param);

/**
 * @brief This function receives DB9 commands coming from connected and authenticated devices and sends to DB9 queue
 *
 * @param this - pointer to bluetooth component instance.
 * @param param : Pointer to event parameters containing the command packet received.
*/
static void __Bluetooth_gattsReceiveDB9Handler(Bluetooth_t * this, esp_ble_gatts_cb_param_t *param);

/**
 * @brief Task code to periodically stimulate the bluetooth state machine with the signals received in the signal queue.
 *
 * @param pvParameters : receives the bluetooth instance containing the state machine stimulated by the task
*/
static void __Bluetooth_taskCode( void * pvParameters );

/**
 * @brief Function that implements the bluetooth state machine.
 *
 * @param this - pointer to bluetooth component instance.
 * @param msg - Signal used to stimulate the state machine.
*/
static bool __Bluetooth_STMStimulate(Bluetooth_t * this, Bluetooth_Signals_t msg);

/************************************************************************************************/
/* State Machine Conditions  Definition */
/************************************************************************************************/

/**
 * @brief This function informs if the bluetooth advertising is enabled
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @return
 * @arg true, if the bluetooth advertising is enabled.
 * @arg false, if the bluetooth advertising is disabled, or an invalid parameter was passed.
 *
*/
static bool __Bluetooth_STM_Condition_advertisingEnabled(Bluetooth_t * this);

/**
 * @brief This function evaluates if the autentication timeout was reached after the AUTHENTICATING state begins.
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @arg true, if the authenticating time is bigger than configured authentication timeout.
 * @arg false, if the authenticating time is less than configured authentication timeout, or an invalid parameter was passed.
*/
static bool __Bluetooth_STM_Condition_authenticationTimeout(Bluetooth_t * this);

/************************************************************************************************/
/* State Machine Actions  Definition */
/************************************************************************************************/

/**
 * @brief  This function calls the BLE GAP API to start the advertising.
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @arg CUBEE_ERROR_OK, if the start advertising command was successfully executed.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __Bluetooth_STM_Action_startAdvertising(Bluetooth_t * this);

/**
 * @brief  This function calls the BLE GAP API to stop the advertising.
 *
 * @arg CUBEE_ERROR_OK, if the stop advertising command was successfully executed.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __Bluetooth_STM_Action_stopAdvertising();

/**
 * @brief  This function calls the BLE GAP API to disconnect from a remote device.
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @arg CUBEE_ERROR_OK, if the disconnection command was successfully executed.
 * @arg CUBEE_ERROR_UNDEFINED, otherwise.
*/
static CubeeErrorCode_t __Bluetooth_STM_Action_disconnectDevice(Bluetooth_t * this);

/**
 * @brief This function sets bluetooth parameters as BLE GAP security settings.
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @arg CUBEE_ERROR_OK, if the configuration was successfully executed.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __Bluetooth_STM_Action_config(Bluetooth_t * this);

/**
 * @brief This function initializes the bluetooth state machine on INITIALIZATION state.
 *
 * @param this - pointer to bluetooth component instance.
 *
 * @arg CUBEE_ERROR_OK, if the state machine was successfully initialized.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
static CubeeErrorCode_t __Bluetooth_STM_Action_init(Bluetooth_t * this);

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

static Bluetooth_t bluetoothInstance =						/*!< Bluetooth instance */
{
		.initialized = false,
		.bluetoothCfg = NULL,
		.bluetoothName = "CUBEE-Energy",
		.semaphore = NULL,
		.peerAddress = {},
		.advertisingStatus = DEFAULT_ADVERTISING_STATUS,
		.gattsProfile =
		{
		        .gatts_cb = __Bluetooth_gattsProfileEventHandler,
		        .gatts_if = ESP_GATT_IF_NONE,
				.app_id = BLUETOOTH_APP_ID,
		},
		.signalQueueHandle = NULL,
		.rcvCfgQueueHandle = NULL,
		.rcvcmdQueueHandle = NULL,
		.stm = {},
};

static uint16_t handleTable[CUBEE_GATTS_ATTRIBUTE_MAX]; 			/*!< Keep the handles to attributes defined in GATT attibute table */
static uint8_t cfgBuffer[CUBEE_CONFIG_SERVICE_VALUE_SIZE] = {0};	/*!< Buffer used to receive and concatenate configuration packets coming from connected devices */
static uint16_t cfgBufferCount = 0;									/*!< Counter used to receive and concatenate configuration packets coming from connected devices */

/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

/**
 * Bluetooth low energy initialization
 */
CubeeErrorCode_t Bluetooth_init(Bluetooth_t * this)
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	if(this == NULL)
	{
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(this->initialized != false)
	{
		/* Already initialized */
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing driver bluetooth...");

	this->bluetoothCfg = Bluetooth_Cfg_getInstance();
	Bluetooth_Cfg_init(this->bluetoothCfg);

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
        ESP_LOGE(LOG_HEADER, "%s enable controller failed", __func__);
        return CUBEE_ERROR_UNDEFINED;
    }

    if (esp_bt_controller_enable(ESP_BT_MODE_BTDM) != ESP_OK) {
        ESP_LOGE(LOG_HEADER, "%s enable controller failed", __func__);
        return CUBEE_ERROR_UNDEFINED;
    }

    if (esp_bluedroid_init() != ESP_OK) {
        ESP_LOGE(LOG_HEADER, "%s init bluedroid failed", __func__);
        return CUBEE_ERROR_UNDEFINED;
    }

    if (esp_bluedroid_enable() != ESP_OK) {
        ESP_LOGE(LOG_HEADER, "%s enable bluedroid failed", __func__);
        return CUBEE_ERROR_UNDEFINED;
    }

    /* Create the command queue */
    this->rcvcmdQueueHandle = xQueueCreate(RCV_CMD_QUEUE_LENGTH, RCV_CMD_QUEUE_ITEM_SIZE);

    /* Create the configuration queue */
    this->rcvCfgQueueHandle = xQueueCreate(CFG_QUEUE_LENGTH, CFG_QUEUE_ITEM_SIZE);

    /* Create the DB9 queue */
     this->rcvDB9QueueHandle = xQueueCreate(DB9_QUEUE_LENGTH, DB9_QUEUE_ITEM_SIZE);

    /* Create state machine signals queue */
     this->signalQueueHandle = xQueueCreate(BLUETOOTH_SIGNAL_QUEUE_LENGTH, BLUETOOTH_SIGNAL_QUEUE_ITEM_SIZE);

	this->semaphore = xSemaphoreCreateMutex();
	if(this->semaphore == NULL)
	{
	  	ESP_LOGE(LOG_HEADER, "Error creating bluetooth semaphore");
	   	return CUBEE_ERROR_UNDEFINED;
	}

     /* Call init function of stm  for instance  */
     __Bluetooth_STM_Action_init(this);

     /* Create the task, storing the handle. */
     xReturned = xTaskCreatePinnedToCore(
     					__Bluetooth_taskCode,      	/* Function that implements the task. */
 						"Bluetooth_task", 			/* Text name for the task. */
 						2048,      					/* Stack size in words, not bytes. */
 						( void * ) this,    		/* Parameter passed into the task. */
 						BLUETOOTH_TASK_PRIORITY,	/* Priority at which the task is created. */
 						&xHandle,					/* Used to pass out the created task's handle. */
						BLUETOOTH_TASK_CORE);     	/* Specifies the core to run the task */
 	if( xReturned != pdPASS )
     {
         return CUBEE_ERROR_UNDEFINED;
     }

    esp_ble_gatts_register_callback(__Bluetooth_gattsEventHandler);
    esp_ble_gap_register_callback(__Bluetooth_gapEventHandler);
    esp_ble_gatts_app_register(BLUETOOTH_APP_ID);

    vTaskDelay(2000/portTICK_PERIOD_MS);

    this->initialized = true;

    ESP_LOGI(LOG_HEADER,"Driver bluetooth initialized");

    return CUBEE_ERROR_OK;
}

Bluetooth_t * Bluetooth_getInstance(void)
{
	return &bluetoothInstance;
}

Bluetooth_Cfg * Bluetooth_getCfg(Bluetooth_t* this)
{
	if(this == NULL)
	{
		return NULL;
	}

	return this->bluetoothCfg;
}

bool Bluetooth_isInitialized(Bluetooth_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

CubeeErrorCode_t Bluetooth_receiveCmd(Bluetooth_t * this, uint8_t * value, uint16_t size)
{

	if((this == NULL) || (value == NULL) || (size < RCV_CMD_QUEUE_ITEM_SIZE))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER reading bluetooth command");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(xQueueReceive(this->rcvcmdQueueHandle,value,RCV_CMD_QUEUE_TIMEOUT) == pdTRUE)
	{
		return CUBEE_ERROR_OK;
	}

	/* No command */
	return CUBEE_ERROR_TIMEOUT;
}

CubeeErrorCode_t Bluetooth_receiveCfg(Bluetooth_t * this, uint8_t * config, uint16_t size)
{
	if((this == NULL) || (config == NULL) || (size < CFG_QUEUE_ITEM_SIZE))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER receiving configuration");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(xQueueReceive(this->rcvCfgQueueHandle,config,CFG_QUEUE_TIMEOUT) == pdTRUE)
	{
#ifdef DEBUG_BLUETOOTH
		ESP_LOGI(LOG_HEADER, "Received configuration from configuration queue: %s", config);
#endif
		return CUBEE_ERROR_OK;
	}

	/* No configuration */
	return CUBEE_ERROR_TIMEOUT;
}

CubeeErrorCode_t Bluetooth_receiveDB9(Bluetooth_t * this, uint8_t * db9Cmd, uint16_t size)
{
	if((this == NULL) || (db9Cmd == NULL) || (size < DB9_QUEUE_ITEM_SIZE))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER receiving DB9 command");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(xQueueReceive(this->rcvDB9QueueHandle,db9Cmd,DB9_QUEUE_TIMEOUT) == pdTRUE)
	{
#ifdef DEBUG_BLUETOOTH
		ESP_LOGI(LOG_HEADER, "Received DB9 from configuration queue: %#02x", *db9Cmd);
#endif
		return CUBEE_ERROR_OK;
	}

	/* No DB9 command */
	return CUBEE_ERROR_TIMEOUT;
}

CubeeErrorCode_t Bluetooth_setAtributeValue(Bluetooth_t * this, Bluetooth_AttributeBDIndex_t attribute, uint8_t * value, uint16_t size)
{
	esp_err_t errorESP;

	if((this == NULL) 	|| (value == NULL) 	|| (size > (this->bluetoothCfg->bluetoothAttributeDB->bluetoothGattsDB[attribute].att_desc.max_length)))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER setting value at attribute handle: %#02x , value: %#02x", handleTable[attribute], (uint8_t)value[0]);
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	errorESP = esp_ble_gatts_set_attr_value(handleTable[attribute],size,(const uint8_t *) value);
	if(errorESP != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_UNDEFINED setting up value at attribute handle: %#02x , value: %#02x", handleTable[attribute], (uint8_t)value[0]);
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

Bluetooth_AdvertisingStatus_t Bluetooth_getAdvertingStatus(Bluetooth_t * this)
{
	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "Error CUBEE_ERROR_INVALID_PARAMETER accessing bluetooth advertising status");
		return BLUETOOTH_ADVERTISING_STATUS_MAX;
	}

	return this->advertisingStatus;
}

CubeeErrorCode_t Bluetooth_setAdvertingStatus(Bluetooth_t * this, Bluetooth_AdvertisingStatus_t status)
{
	if((this == NULL) 	|| (status > BLUETOOTH_ADVERTISING_STATUS_MAX))
	{
		ESP_LOGE(LOG_HEADER, "Error CUBEE_ERROR_INVALID_PARAMETER setting advertising status");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Updates the advertising status */
	this->advertisingStatus = status;

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t Bluetooth_sendSignal(Bluetooth_t * this, Bluetooth_Signals_t rcvSignal)
{
	Bluetooth_Signals_t signal = BLUETOOTH_NOSIG;

	if((this == NULL) || (rcvSignal >= BLUETOOTH_SIGNAL_MAX))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER sending signal to Bluetooth");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if( xSemaphoreTake( this->semaphore, BLUETOOTH_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking bluetooth semaphore");
		return CUBEE_ERROR_TIMEOUT;
	}

	switch(rcvSignal)
	{
	case BLUETOOTH_SIGNAL_INITIALIZED:
		if(this->stm.mainState.activeSubState == BLUETOOTH_STATE_INITIALIZATION)
		{
	    	signal = BLUETOOTH_SIGNAL_INITIALIZED;
		}
		break;
	case BLUETOOTH_SIGNAL_CONFIGURED:
		if(this->stm.mainState.activeSubState == BLUETOOTH_STATE_CONFIGURATION)
		{
	    	signal = BLUETOOTH_SIGNAL_CONFIGURED;
		}
		break;
	case BLUETOOTH_SIGNAL_AUTHENTICATED:
		if(this->stm.mainState.activeSubState == BLUETOOTH_STATE_AUTHENTICATING)
		{
	    	signal = BLUETOOTH_SIGNAL_AUTHENTICATED;
		}
		break;
	case BLUETOOTH_SIGNAL_ADVERTISING_STOPPED:
		if(this->stm.mainState.activeSubState == BLUETOOTH_STATE_ADVERTISING)
		{
	    	signal = BLUETOOTH_SIGNAL_ADVERTISING_STOPPED;
		}
		break;
	case BLUETOOTH_SIGNAL_ADVERTISNG_STARTED:
		if(this->stm.mainState.activeSubState == BLUETOOTH_STATE_IDLE)
		{
	    	signal = BLUETOOTH_SIGNAL_ADVERTISNG_STARTED;
		}
		break;
	case BLUETOOTH_SIGNAL_DEVICE_CONNECTED:
		if(this->stm.mainState.activeSubState == BLUETOOTH_STATE_ADVERTISING)
		{
	    	signal = BLUETOOTH_SIGNAL_DEVICE_CONNECTED;
		}
		break;
	case BLUETOOTH_SIGNAL_DEVICE_DISCONNECTED:
		if((this->stm.mainState.activeSubState == BLUETOOTH_STATE_AUTHENTICATING) || (this->stm.mainState.activeSubState == BLUETOOTH_STATE_AUTHENTICATED))
		{
	    	signal = BLUETOOTH_SIGNAL_DEVICE_DISCONNECTED;
		}
		break;
	default:
		break;
	}

	if(signal != BLUETOOTH_NOSIG)
	{
		/* Send the bluetooth signal to the state machine signal queue*/
		if(xQueueGenericSend(this->signalQueueHandle, &signal, BLUETOOTH_SIGNAL_QUEUE_TIMEOUT,queueSEND_TO_BACK) != pdTRUE)
		{
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			xSemaphoreGive(this->semaphore);
			ESP_LOGE(LOG_HEADER,"Error sending signal, queue full");
			return CUBEE_ERROR_UNDEFINED;
		}
	}

	/* We have finished accessing the shared resource.  Release the	 semaphore. */
	xSemaphoreGive(this->semaphore);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t Bluetooth_setName(Bluetooth_t * this, uint8_t * name)
{
	if((this == NULL) || (name == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER setting up the bluetooth advertising name ");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	memset(this->bluetoothName,0,CUBEE_NAME_MAX_SIZE);
	memmove(this->bluetoothName,name,(size_t)strlen((char *)name));

	if((esp_ble_gap_set_device_name((const char *)&this->bluetoothName)!= ESP_OK)
		|| (esp_ble_gap_config_adv_data(&this->bluetoothCfg->bluetoothAdvData) != ESP_OK))
	{
		ESP_LOGE(LOG_HEADER,"Error updating bluetooth name");
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER,"Bluetooth name updated to %s:", name);

	return CUBEE_ERROR_OK;

}
/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

static void __Bluetooth_taskCode( void * pvParameters )
{
	Bluetooth_t * bluetooth = (Bluetooth_t *) pvParameters;
	Bluetooth_Signals_t rcvSignal;

    for( ;; )
    {

    	/* Get signal from queue */
    	if(xQueueReceive(bluetooth->signalQueueHandle,&rcvSignal,BLUETOOTH_SIGNAL_QUEUE_TIMEOUT) == pdTRUE)
    	{
    		/* Stimulates the state machine with signal received */
    		__Bluetooth_STMStimulate(bluetooth, rcvSignal);
    	}
    	else
    	{
    		__Bluetooth_STMStimulate(bluetooth, BLUETOOTH_NOSIG);
    	}

		/* Block for 100ms. */
		 vTaskDelay( BLUETOOTH_TASK_DELAY );
    }
}

static bool __Bluetooth_STMStimulate(Bluetooth_t * this, Bluetooth_Signals_t msg)
{
    bool evConsumed = false;

    switch (this->stm.mainState.activeSubState)
    {

    case BLUETOOTH_STATE_INITIALIZATION:
    	/* Verify transition conditions */
    	if(msg == BLUETOOTH_SIGNAL_INITIALIZED)
    	{
    		evConsumed = true;

    		/* transition actions */
    		__Bluetooth_STM_Action_config(this);

    		/* transition */
    		this->stm.mainState.activeSubState = BLUETOOTH_STATE_CONFIGURATION;
    		ESP_LOGI(LOG_HEADER,"Transition, BLUETOOTH_STATE_INITIALIZATION -> BLUETOOTH_STATE_CONFIGURATION, task priority: %d, task core: %d", uxTaskPriorityGet(NULL), xPortGetCoreID());
    	}
    	break;

    case BLUETOOTH_STATE_CONFIGURATION:
    	/* Verify transition conditions */
    	if(msg == BLUETOOTH_SIGNAL_CONFIGURED)
    	{
    		evConsumed = true;

    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = BLUETOOTH_STATE_IDLE;
    		ESP_LOGI(LOG_HEADER,"Transition, BLUETOOTH_STATE_CONFIGURATION -> BLUETOOTH_STATE_IDLE");
    	}
    	break;

    case BLUETOOTH_STATE_IDLE:
    	/* Verify transition conditions */
    	if(msg == BLUETOOTH_SIGNAL_ADVERTISNG_STARTED)
    	{
    		evConsumed = true;
    		/* transition actions */

    		/* transition */
    		this->stm.mainState.activeSubState = BLUETOOTH_STATE_ADVERTISING;
    		ESP_LOGI(LOG_HEADER,"Transition, BLUETOOTH_STATE_IDLE -> BLUETOOTH_STATE_ADVERTISING");
    	}
    	else if(__Bluetooth_STM_Condition_advertisingEnabled(this))
    	{
    		__Bluetooth_STM_Action_startAdvertising(this);
    	}

        break;

    case BLUETOOTH_STATE_ADVERTISING:
       	if(msg == BLUETOOTH_SIGNAL_DEVICE_CONNECTED)
        {
        	evConsumed = true;
        	/* transition actions */

        		/* transition */
        	this->stm.mainState.activeSubState = BLUETOOTH_STATE_AUTHENTICATING;
       		FSM_startTime(&(this->stm.Authenticating));
       		ESP_LOGI(LOG_HEADER,"Transition, BLUETOOTH_STATE_ADVERTISING -> BLUETOOTH_STATE_AUTHENTICATING");
       	}
       	else if(msg == BLUETOOTH_SIGNAL_ADVERTISING_STOPPED)
       	{
       		evConsumed = true;
        	/* transition actions */

        	/* transition */
       		this->stm.mainState.activeSubState = BLUETOOTH_STATE_IDLE;
       		ESP_LOGI(LOG_HEADER,"Transition, BLUETOOTH_STATE_ADVERTISING -> BLUETOOTH_STATE_IDLE");
       	}
    	else if(!__Bluetooth_STM_Condition_advertisingEnabled(this))
    	{
    		__Bluetooth_STM_Action_stopAdvertising();
    	}

        break;

    case BLUETOOTH_STATE_AUTHENTICATING:
       	if(msg == BLUETOOTH_SIGNAL_DEVICE_DISCONNECTED)
        {
        	evConsumed = true;
        	/* transition actions */

        		/* transition */
        	this->stm.mainState.activeSubState = BLUETOOTH_STATE_IDLE;
       		ESP_LOGI(LOG_HEADER,"Transition, BLUETOOTH_STATE_AUTHENTICATING -> BLUETOOTH_STATE_IDLE");
       	}
       	else if(msg == BLUETOOTH_SIGNAL_AUTHENTICATED)
       	{
           	/* transition actions */

            /* transition */
       		this->stm.mainState.activeSubState = BLUETOOTH_STATE_AUTHENTICATED;
           	ESP_LOGI(LOG_HEADER,"Transition, BLUETOOTH_STATE_AUTHENTICATING -> BLUETOOTH_STATE_AUTHENTICATED");
       	}
       	else if(__Bluetooth_STM_Condition_authenticationTimeout(this))
       	{
       		__Bluetooth_STM_Action_disconnectDevice(this);
       	}

        break;

    case BLUETOOTH_STATE_AUTHENTICATED:
       	if(msg == BLUETOOTH_SIGNAL_DEVICE_DISCONNECTED)
        {
        	evConsumed = true;
        	/* transition actions */

        		/* transition */
        	this->stm.mainState.activeSubState = BLUETOOTH_STATE_IDLE;
       		ESP_LOGI(LOG_HEADER,"Transition, BLUETOOTH_STATE_AUTHENTICATED -> BLUETOOTH_STATE_IDLE");
       	}

        break;

    default:
        break;
    }

    return evConsumed;
}

static void __Bluetooth_gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
#ifdef DEBUG_BLUETOOTH
	ESP_LOGI(LOG_HEADER, "GAP_EVT, event %d, task priority: %d, task core: %d", event, uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_GPIO */

    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
#ifdef DEBUG_BLUETOOTH
    	ESP_LOGI(LOG_HEADER, "ADVERTISING_DATA_SET_EVT");
#endif /* DEBUG_BLUETOOTH */
       	/* Send the BLUETOOTH_CONFIGURED signal to the state machine signal queue*/
        Bluetooth_sendSignal(&bluetoothInstance, BLUETOOTH_SIGNAL_CONFIGURED);
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
    	ESP_LOGI(LOG_HEADER, "ADVERTISING_START_COMPLETE_EVT");
        /* advertising start complete event to indicate advertising start successfully or failed */
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(LOG_HEADER, "Advertising start failed");
        }
        else
        {
           	/* Send the ADVERTISNG_STARTED signal to the state machine signal queue*/
            Bluetooth_sendSignal(&bluetoothInstance, BLUETOOTH_SIGNAL_ADVERTISNG_STARTED);
        }
        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
    	ESP_LOGI(LOG_HEADER, "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT");
    	/* Send the ADVERTISNG_STOPPED signal to the state machine signal queue*/
    	Bluetooth_sendSignal(&bluetoothInstance, BLUETOOTH_SIGNAL_ADVERTISING_STOPPED);
        break;

    case ESP_GAP_BLE_SEC_REQ_EVT:
#ifdef DEBUG_BLUETOOTH
    	ESP_LOGI(LOG_HEADER, "SECURITY_REQUEST_EVT ");
#endif /* DEBUG_BLUETOOTH */
            /* send the positive(true) security response to the peer device to accept the security request.
            If not accept the security request, should sent the security response with negative(false) accept value*/
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, false);
            break;

    case ESP_GAP_BLE_KEY_EVT:
#ifdef DEBUG_BLUETOOTH
        /*shows the ble key info share with peer device to the user.*/
        ESP_LOGI(LOG_HEADER, "key type = %d", param->ble_security.ble_key.key_type);
#endif /* DEBUG_GPIO */

        break;

    case ESP_GAP_BLE_AUTH_CMPL_EVT:
    {
        ESP_LOGI(LOG_HEADER, "Authentication complete, remote %02x:%02x:%02x:%02x:%02x:%02x, pairing status = %s",
        		param->ble_security.auth_cmpl.bd_addr[0], param->ble_security.auth_cmpl.bd_addr[1],param->ble_security.auth_cmpl.bd_addr[2],
				param->ble_security.auth_cmpl.bd_addr[3], param->ble_security.auth_cmpl.bd_addr[4],param->ble_security.auth_cmpl.bd_addr[5],
				param->ble_security.auth_cmpl.success ? "success" : "fail");
        break;
    }

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(LOG_HEADER, "update connetion params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
		break;

    default:
        break;
    }
}

static void __Bluetooth_gattsEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
#ifdef DEBUG_BLUETOOTH
	ESP_LOGI(LOG_HEADER, "GATTS_EVT, = %d, task priority: %d, task core: %d", event, uxTaskPriorityGet(NULL), xPortGetCoreID());
#endif /* DEBUG_GPIO */

    /* Forwards the event to the right GATT server profile callback */
    if (((event == ESP_GATTS_REG_EVT) &&  (param->reg.app_id == bluetoothInstance.gattsProfile.app_id)) ||
    		(gatts_if == bluetoothInstance.gattsProfile.gatts_if))
    {
    	bluetoothInstance.gattsProfile.gatts_cb(event, gatts_if, param);
    }
}

static void __Bluetooth_gattsProfileEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:
#ifdef DEBUG_BLUETOOTH
        	ESP_LOGI(LOG_HEADER, "REGISTER_APP_EVT, status %d, app_id %d", param->reg.status, param->reg.app_id);
#endif /* DEBUG_GPIO */
        	if ((param->reg.status == ESP_GATT_OK))
        	{
        	  	bluetoothInstance.gattsProfile.gatts_if = gatts_if;	/*Store the GATT interface used by this GATT server profile*/
        	  	esp_ble_gatts_create_attr_tab(bluetoothInstance.bluetoothCfg->bluetoothAttributeDB->bluetoothGattsDB, gatts_if, CUBEE_GATTS_ATTRIBUTE_MAX, CUBEE_BLUETOOTH_SVC_INSTANCE_ID);
        	}
        	else
        	{
        		ESP_LOGE(LOG_HEADER, "Reg app failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
        	}

            break;

        case ESP_GATTS_UNREG_EVT:
 #ifdef DEBUG_BLUETOOTH
         	ESP_LOGI(LOG_HEADER, "UNREGISTER_APP_ID_EVT");
 #endif /* DEBUG_BLUETOOTH */
         	bluetoothInstance.gattsProfile.gatts_if = ESP_GATT_IF_NONE;
         	esp_ble_gatts_app_register(bluetoothInstance.gattsProfile.app_id);
         	break;

        case ESP_GATTS_READ_EVT:
#ifdef DEBUG_BLUETOOTH
        	ESP_LOGI(LOG_HEADER, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %#04x", param->read.conn_id, param->read.trans_id, param->read.handle);
#endif /* DEBUG_BLUETOOTH */
            break;

        case ESP_GATTS_WRITE_EVT:
#ifdef DEBUG_BLUETOOTH
        	ESP_LOGI(LOG_HEADER, "GATT_WRITE_EVT, handle %#04x, value len %d", param->write.handle, param->write.len);
#endif /* DEBUG_BLUETOOTH */
        	/* Verifies if the peer device is authenticated */
        	if(bluetoothInstance.stm.mainState.activeSubState != BLUETOOTH_STATE_AUTHENTICATED)
        	{
        		ESP_LOGE(LOG_HEADER, "The write command can't be executed, DEVICE NOT AUTHENTICATED");
        	}
        	else if(handleTable[CUBEE_RCV_CMD_VAL_IDX]	== (uint16_t) (param->write.handle))
        	{
        		/* The data received is a command */
        		__Bluetooth_gattsReceiveCmdHandler(&bluetoothInstance, param);
        	}
        	else if (handleTable[CUBEE_CONFIG_VAL_IDX] == (uint16_t) (param->write.handle))
        	{
        		/* The data received is a configuration block */
        		__Bluetooth_gattsReceiveCfgHandler(&bluetoothInstance, param);
        	}
          	else if(handleTable[CUBEE_DB9_VAL_IDX]	== (uint16_t) (param->write.handle))
            {
          		/* The data received is a db9 command */
          		__Bluetooth_gattsReceiveDB9Handler(&bluetoothInstance, param);
            }
        	break;

        case ESP_GATTS_START_EVT:
#ifdef DEBUG_BLUETOOTH
        	ESP_LOGI(LOG_HEADER, "START_SERVICE_EVT, service status: %d service handle: %#04x", param->start.status, param->start.service_handle);
#endif /* DEBUG_BLUETOOTH */
           	/* The bluetooth service was successfully started, send the BLUETOOTH_SIGNAL_INITIALIZED signal to the state machine signal queue*/
            Bluetooth_sendSignal(&bluetoothInstance, BLUETOOTH_SIGNAL_INITIALIZED);
            break;

        case ESP_GATTS_CONNECT_EVT:
        	ESP_LOGI(LOG_HEADER, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:, connection status: %d",
        	        	                 param->connect.conn_id,
        	        	                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
        	        	                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5],
        	        	                 param->connect.is_connected);
            /* start security connect with peer device when receive the connect event sent by the master */
            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_NONE);
        	/* Copy remote device address and send the CONNECT signal to the state machine signal queue*/
        	memmove(bluetoothInstance.peerAddress,param->connect.remote_bda,ESP_BD_ADDR_LEN);
        	Bluetooth_sendSignal(&bluetoothInstance, BLUETOOTH_SIGNAL_DEVICE_CONNECTED);
        	break;

        case ESP_GATTS_DISCONNECT_EVT:
        	ESP_LOGI(LOG_HEADER, "DISCONNECT, connection status: %d", param->disconnect.is_connected);

            /* Clean remote device address and send the DISCONNECT signal to the state machine signal queue */
            memset(bluetoothInstance.peerAddress,0,ESP_BD_ADDR_LEN);
            Bluetooth_sendSignal(&bluetoothInstance, BLUETOOTH_SIGNAL_DEVICE_DISCONNECTED);

            break;

        case ESP_GATTS_CREAT_ATTR_TAB_EVT:
#ifdef DEBUG_BLUETOOTH

            for(int i = 0; i < param->add_attr_tab.num_handle; i++)
            {
            	ESP_LOGI(LOG_HEADER, "Atribute Handle[%d] = %d ",i,param->add_attr_tab.handles[i]);
            }
#endif /* DEBUG_BLUETOOTH */
        	ESP_LOGI(LOG_HEADER, "CREATE_ATTRIBUTE_TABLE_EVT, handle =%#04x",param->add_attr_tab.num_handle);
            if (param->add_attr_tab.status != ESP_GATT_OK){
                ESP_LOGE(LOG_HEADER, "Create attribute table failed, error code=0x%x", param->add_attr_tab.status);
            }
            else if (param->add_attr_tab.num_handle != CUBEE_GATTS_ATTRIBUTE_MAX){
                ESP_LOGE(LOG_HEADER, "Create attribute table abnormally, num_handle (%d) doesn't equal to the number of attributes", param->add_attr_tab.num_handle);
            }
            else
            {
            	memcpy(handleTable, param->add_attr_tab.handles, sizeof(handleTable));
                esp_ble_gatts_start_service(handleTable[CUBEE_SVC_IDX]);
            }
            break;

#ifdef DEBUG_BLUETOOTH
        case ESP_GATTS_SET_ATTR_VAL_EVT:
        	ESP_LOGI(LOG_HEADER, "ESP_GATTS_SET_ATTR_VAL_EVT, attr_handle %#04x, srvc_handle %#04x, status %d ", param->set_attr_val.attr_handle, param->set_attr_val.srvc_handle, param->set_attr_val.status);
        	break;

        case ESP_GATTS_EXEC_WRITE_EVT:
        	ESP_LOGI(LOG_HEADER, "EXECUTE_WRITE_EVT");
            break;

        case ESP_GATTS_CREATE_EVT:
        	ESP_LOGI(LOG_HEADER, "CREATE_SERVICE_EVT, Service Handle:  %#04x Service Id: %#04x Service Status: %d", param->create.service_handle,param->create.service_id.id.uuid.uuid.uuid16,param->create.status);
        break;

        case ESP_GATTS_ADD_CHAR_EVT:
        	ESP_LOGI(LOG_HEADER, "ADD_CHARACTERISTIC_EVT, Attribute Handle:  %#04x Service Handle:  %#04x Characteristic UUID:  %#04x", param->add_char.attr_handle,param->add_char.service_handle,  param->add_char.char_uuid.uuid.uuid16);
        break;

        case ESP_GATTS_MTU_EVT:
        	ESP_LOGI(LOG_HEADER, "MTU_COMPLETE_EVT");
            break;

        case ESP_GATTS_CONF_EVT:
        	ESP_LOGI(LOG_HEADER, "CONFIRM_EVT");
            break;

        case ESP_GATTS_DELETE_EVT:
        	ESP_LOGI(LOG_HEADER, "SERVICE_DELETE_EVT");
            break;

        case ESP_GATTS_STOP_EVT:
        	ESP_LOGI(LOG_HEADER, "START_SERVICE_EVT, service status: %d service handle: %#04x", param->stop.status, param->stop.service_handle);
            break;

        case ESP_GATTS_OPEN_EVT:
        	ESP_LOGI(LOG_HEADER, "OPEN_EVT");
            break;

        case ESP_GATTS_CANCEL_OPEN_EVT:
        	ESP_LOGI(LOG_HEADER, "CANCEL_OPEN_EVT");
            break;

        case ESP_GATTS_CLOSE_EVT:
        	ESP_LOGI(LOG_HEADER, "CLOSE_EVT");
            break;

        case ESP_GATTS_LISTEN_EVT:
        	ESP_LOGI(LOG_HEADER, "LISTEN_EVT");
            break;

        case ESP_GATTS_CONGEST_EVT:
        	ESP_LOGI(LOG_HEADER, "CONGEST_EVT");
            break;
#endif /* DEBUG_BLUETOOTH */

        default:
           break;
    }
}

static void __Bluetooth_gattsReceiveCfgHandler(Bluetooth_t * this, esp_ble_gatts_cb_param_t *param)
{
	uint8_t * endSequencePointer;
	uint8_t * initSequencePointer;

	/* Verify if the packet is the first */
	initSequencePointer = (uint8_t *) strchr((char *) param->write.value, '{');
	if(initSequencePointer != NULL)
	{
		/* First character detected, Clear config buffer and reset configuration packet counter to receive the configuration */
		memset(cfgBuffer,0,CUBEE_CONFIG_SERVICE_VALUE_SIZE);
		cfgBufferCount = 0;
	}

	if((cfgBufferCount + param->write.len) > CUBEE_CONFIG_SERVICE_VALUE_SIZE)
	{
		ESP_LOGE(LOG_HEADER, "Failed receiving configuration, MAX_CONFIG_SIZE exceeded");
		/*Clear config buffer and reset configuration packet counter*/
		memset(cfgBuffer,0,CUBEE_CONFIG_SERVICE_VALUE_SIZE);
		cfgBufferCount = 0;
		return;
	}

	/* Add the packet to the buffer */
	memmove(&cfgBuffer[cfgBufferCount], param->write.value, param->write.len);
#ifdef DEBUG_BLUETOOTH
	ESP_LOGI(LOG_HEADER, "COnfiguration packet received, size: %d ", param->write.len);
#endif
	/* Updates packet count */
	cfgBufferCount += param->write.len;

	/* Verify if the packet is the last */
	endSequencePointer = (uint8_t *) strstr((char *) cfgBuffer,"}");
	if(endSequencePointer != NULL)
	{
		/* Write the configuration to the configuration queue */
		xQueueOverwrite(this->rcvCfgQueueHandle, cfgBuffer);
		ESP_LOGI(LOG_HEADER, "COnfiguration received and added to configuration queue, size: %d\nconfig: %s", cfgBufferCount, (char * ) cfgBuffer);

		/*Clear config buffer and reset configuration packet counter*/
		memset(cfgBuffer,0,CUBEE_CONFIG_SERVICE_VALUE_SIZE);
		cfgBufferCount = 0;
	}
}

static void __Bluetooth_gattsReceiveCmdHandler(Bluetooth_t * this, esp_ble_gatts_cb_param_t *param)
{
	ESP_LOGI(LOG_HEADER, "Command received and added to command queue, %#04x", param->write.value[0]);
	xQueueOverwrite(this->rcvcmdQueueHandle, (uint8_t * )param->write.value);
}

static void __Bluetooth_gattsReceiveDB9Handler(Bluetooth_t * this, esp_ble_gatts_cb_param_t *param)
{
	ESP_LOGI(LOG_HEADER, "DB9 Command received and added to DB9 queue, %#02x", param->write.value[0]);
	xQueueOverwrite(this->rcvDB9QueueHandle, (uint8_t * )param->write.value);
}

/************************************************************************************************/
/**************************** State Machine Conditions  Implementation***************************/
/************************************************************************************************/

static bool __Bluetooth_STM_Condition_advertisingEnabled(Bluetooth_t * this)
{
	return (this->advertisingStatus == BLUETOOTH_ADVERTISING_ENABLED) ? true : false;
}

static bool __Bluetooth_STM_Condition_authenticationTimeout(Bluetooth_t * this)
{
	uint64_t timeElapsed = FSM_getTimeElapsedasd(&(this->stm.Authenticating), TIME_MILLISECONDS);

	return  (timeElapsed > BLUETOOTH_AUTHENTICATION_TIMEOUT_MS);
}

/************************************************************************************************/
/******************************* State Machine Actions  Implementation **************************/
/************************************************************************************************/

static CubeeErrorCode_t __Bluetooth_STM_Action_startAdvertising(Bluetooth_t * this)
{
	return (esp_ble_gap_start_advertising(&this->bluetoothCfg->bluetoothAdvParams) == ESP_OK) ? CUBEE_ERROR_OK : CUBEE_ERROR_UNDEFINED;
}

static CubeeErrorCode_t __Bluetooth_STM_Action_stopAdvertising()
{
	return (esp_ble_gap_stop_advertising() == ESP_OK) ? CUBEE_ERROR_OK : CUBEE_ERROR_UNDEFINED;
}

static CubeeErrorCode_t __Bluetooth_STM_Action_disconnectDevice(Bluetooth_t * this)
{
	return (esp_ble_gap_disconnect(this->peerAddress) == ESP_OK) ? CUBEE_ERROR_OK : CUBEE_ERROR_UNDEFINED;;
}

static CubeeErrorCode_t __Bluetooth_STM_Action_config(Bluetooth_t * this)
{
    /* Security settings : set the security iocap & auth_req & key size & init key response key parameters to the stack*/
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_NO_BOND;     /* Doesn't bonding with peer device after authentication */
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           /* set the IO capability to No output No input */
    uint8_t key_size = 16;      						/* the key size should be 7~16 bytes */
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __Bluetooth_STM_Action_init(Bluetooth_t * this)
{
    /* State Idle doesn't have substate*/
    this->stm.Idle.activeSubState = BLUETOOTH_NOSTATE;

    /* State Initialization doesn't have substate */
    this->stm.Initialization.activeSubState = BLUETOOTH_NOSTATE;

    /* State Advertising doesn't have substate */
    this->stm.Advertising.activeSubState = BLUETOOTH_NOSTATE;

    /* State Authenticating doesn't have substate */
    this->stm.Authenticating.activeSubState = BLUETOOTH_NOSTATE;

    /* State Authenticated doesn't have substate */
    this->stm.Authenticated.activeSubState = BLUETOOTH_NOSTATE;

    /* Initial State -> Initialization */
    this->stm.mainState.activeSubState = BLUETOOTH_STATE_INITIALIZATION;
    FSM_startTime(&(this->stm.Initialization));

	return CUBEE_ERROR_OK;
}
