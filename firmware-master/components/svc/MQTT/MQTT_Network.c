/**
 * @file MQTT_Network.h
 * @version 1.0
 * @author Alex Fernandes
 * @date October 16, 2017
 **************************************************************************
 *
 * @brief  MQTT Network service
 * This module implements the communication over a MQTT Network. The module
 * implements a MQTT client that communicates with a MQTT Broker pre-defined.
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   16-Oct-2017    Alex Fernandes
 * * Original version
 ******************************************************************/

#include "MQTT_Network.h"

#define LOG_HEADER 						"[MQTT NETWORK] "
#define WAITING_WIFI_CONNECTION_TIMEOUT		(5000)	/* 5 seconds */

/* Constants that aren't configurable in menuconfig */
//#define MQTT_SERVER "192.168.0.110"		/* Local */
#define MQTT_SERVER "150.165.85.21"			/* Server */
//#define MQTT_PORT 1883 /* Standard connection */
#define MQTT_PORT 8883	/* SSL connection */
//#define MQTT_PORT 8084	/* WebSocket/SSL connection */

#define MQTT_NETWORK_SIGNAL_QUEUE_LENGTH		(1)								/*!< Length of queue used to store subscription update signals */
#define MQTT_NETWORK_SIGNAL_QUEUE_ITEM_SIZE  	(sizeof(uint8_t))				/*!< Size of update signals */
#define MQTT_NETWORK_SIGNAL_QUEUE_TIMEOUT		(1 / portTICK_PERIOD_MS)		/*!< Waiting timeout to receive update signals from queue */
#define MQTT_NETWORK_SEMAPHORE_TIMEOUT			(3000 / portTICK_PERIOD_MS)		/*!< Waiting timeout to access MQTT shared resources */

#define MAX_CLIENT_ID_SIZE						(128)

/* Random number generator register */
#define DR_REG_RNG_BASE                        0x3ff75144

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

static unsigned char mqtt_sendBuf[MQTT_BUF_SIZE];
static unsigned char mqtt_readBuf[MQTT_BUF_SIZE];
static MQTTMessage message;
static bool updateConnection = false;
static char clientIDBuffer[MAX_CLIENT_ID_SIZE];
MQTTString clientId = MQTTString_initializer;

static MQTT_Network_t mqttNetworkInstance =
{
		.initialized = false,				/*!< Indicates if the module instance was initialized */
		.wifiController = NULL,				/*!< Pointer to wifi controller used by the module instance */
		.subscriptionUpdate = NULL,			/*!< Queues used to store requests for subscriptions update */
		.subscriptionsTopics = {},			/*!< Topics used in subscriptions */
		.subscriptionCount = 0,				/*!< Counter for number of subscriptions */
		.receiveDataCallback = NULL,		/*!< callback used to notify about data received over MQTT network */
		.client = {0},						/*!< client MQTT */
		.network = {0},						/*!< Network used to connect the MWTT client */
		.data = MQTTPacket_connectData_initializer,						/*!< Settings to MQTT client connection  */
		.broker_address = MQTT_SERVER,		/*!< Address to MQTT Broker connection  */
		.broker_port = MQTT_PORT,			/*!< Port to MQTT Broker connection  */
		.clientId = MQTTString_initializer,
		.semaphore = NULL,					/*!< Semaphore used to synchronize access to MQTT Network shared resources */
};

/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

/**
 * Task for implementation of MQTT network.
 *
 * @param pvParameters - Pointer to module instance
 */
static void __MQTT_Network_task(void *pvParameters);

/**
 * @brief This function does the the client subscriptions in all topics defined in
 * the module attribute  subscriptionsTopics
 *
 * @param this - module instance
 *
 * @return
 * @param CUBEE_ERROR_OK - if all subscriptions were successfully done
 * @param CUBEE_ERROR_CODE - otherwise
 */
static CubeeErrorCode_t __MQTT_Network_subscribe(MQTT_Network_t * this);

/**
 * @brief Callback to receive MQTT data
 *
 * @param md - message received
 */
static void __MQTT_Network_message_handler(MessageData *md);



/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t MQTT_Network_init(MQTT_Network_t * this)
{
	BaseType_t xReturned;
	TaskHandle_t xHandle = NULL;

	if(this == NULL)
	{
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Verify if the module instance is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing MQTT Network APP... ");

	this->wifiController = Wifi_Ctrl_getInstance();
    if (Wifi_Ctrl_init(this->wifiController) != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "Wifi controller initialization failed");
        return CUBEE_ERROR_UNDEFINED;
    }

    /* Initialize network */
    MQTTmbedtls_initNetwork(&this->network, false, false);

    /* Initializes client connection data */
	this->data.willFlag          = 0;
	this->data.MQTTVersion       = 3; // 3 = 3.1 4 = 3.1.1
	this->data.keepAliveInterval = 120;
	this->data.cleansession      = 1;

    /* Create update signal queue */
    this->subscriptionUpdate = xQueueCreate(MQTT_NETWORK_SIGNAL_QUEUE_LENGTH, MQTT_NETWORK_SIGNAL_QUEUE_ITEM_SIZE);
	if(this->subscriptionUpdate == NULL)
	{
	  	ESP_LOGE(LOG_HEADER, "Error creating MQTT Network subscription update queue");
	   	return CUBEE_ERROR_UNDEFINED;
	}

    /* Create semaphore to access MQTT Network shared resources */
	this->semaphore = xSemaphoreCreateMutex();
	if(this->semaphore == NULL)
	{
	  	ESP_LOGE(LOG_HEADER, "Error creating MQTT Network semaphore");
	   	return CUBEE_ERROR_UNDEFINED;
	}

     /* Create the task, storing the handle. */
     xReturned = xTaskCreatePinnedToCore(
    		 	 	 	__MQTT_Network_task,      	/* Function that implements the task. */
 						"mqtt_Network_task", 		/* Text name for the task. */
						12288,     					/* Stack size in words, not bytes. */
 						( void * ) this,    		/* Parameter passed into the task. */
						MQTT_TASK_PRIORITY,			/* Priority at which the task is created. */
 						&xHandle,					/* Used to pass out the created task's handle. */
						MQTT_TASK_CORE);     		/* Specifies the core to run the task */
 	if( xReturned != pdPASS )
     {
         return CUBEE_ERROR_UNDEFINED;
     }

    this->initialized = true;

    ESP_LOGI(LOG_HEADER,"MQTT Network successfully initialized ");

    return CUBEE_ERROR_OK;
}

MQTT_Network_t * MQTT_Network_getInstance(void)
{
	return &mqttNetworkInstance;
}

CubeeErrorCode_t MQTT_Network_sendData(MQTT_Network_t * this, MQTT_Network_Data_Packet_t * data, bool acknowledgment)
{
	int result = SUCCESS;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER sending data over MQTT Network");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(!this->client.isconnected)
	{
		ESP_LOGE(LOG_HEADER,"Error sending data over MQTT Network, client isn't connected");
		return CUBEE_ERROR_UNDEFINED;
	}

	if( xSemaphoreTake( this->semaphore, MQTT_NETWORK_SEMAPHORE_TIMEOUT ) != pdTRUE )
	{
		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
		ESP_LOGE(LOG_HEADER,"Error taking MQTT Network semaphore");
		return CUBEE_ERROR_TIMEOUT;
	}

	/* Build and publish message */
	message.retained = false;
	message.dup = false;
	message.payload = (void*)data->dataBuffer;
	message.payloadlen = data->dataBufferSize;

	/* Set quality of service */
	if(acknowledgment)
	{
		message.qos = QOS1;
	}
	else
	{
		message.qos = QOS0;
	}

	result = MQTTClient_Publish(&this->client, data->topic, &message);
	if (result != SUCCESS)
	{
		ESP_LOGE(LOG_HEADER, "Error sending data over MQTT Network");
		if(result == FAILURE)
		{
			/* Send connection update request */
			MQTT_Network_updateSubscriptions(this);
		}
		xSemaphoreGive(this->semaphore);
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER, "Data successfully sent over MQTT network, topic: %s, data: %s", data->topic, (char *)data->dataBuffer);
	xSemaphoreGive(this->semaphore);
	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t MQTT_Network_updateSubscriptions(MQTT_Network_t * this)
{
	uint8_t signal = 1;

	if((this == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER sending subscription update request");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Overwrite the rule in the queue */
	if(xQueueOverwrite(this->subscriptionUpdate, &signal) != pdTRUE)
	{
		ESP_LOGE(LOG_HEADER,"Error sending subscription update request");
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER, "Update subscription signal successfully sent");

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t MQTT_Network_registerRcvCallback(MQTT_Network_t * this, mqtt_rcv_cb_t callback)
{
	if((this == NULL) || (callback == NULL))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER registering callback");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	this->receiveDataCallback = callback;

	return CUBEE_ERROR_OK;
}

bool MQTT_Network_isConnected(MQTT_Network_t * this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER verifying if network is connected");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	return this->client.isconnected;
}

/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

static void __MQTT_Network_task(void *pvParameters)
{
	uint8_t updateSubscriptionSignal;
	uint32_t randomNumber;
	bool yieldFailed;
	bool updateSubscriptionReqest;

	MQTT_Network_t * this = (MQTT_Network_t *) pvParameters;

    while(true)
    {
    	ESP_LOGI(LOG_HEADER,"Updating MQTT connection...");
		updateConnection = false;
    	while ( xSemaphoreTake( this->semaphore, MQTT_NETWORK_SEMAPHORE_TIMEOUT ) != pdTRUE )
    	{
    		/* We could not obtain the semaphore and can therefore not access the shared resource safely. */
    		vTaskDelay( MQTT_TASK_DELAY );
    	}

        /* Initilize MQTT client */
        MQTTClient_Init(&this->client, &this->network,
    		2000,          /*!< command_timeout_ms */
    		mqtt_sendBuf,  /*!< sendbuf */
    		MQTT_BUF_SIZE, /*!< sendbuf_size */
    		mqtt_readBuf,  /*!< readbuf */
    		MQTT_BUF_SIZE  /*!< readbuf_size */
    	);

    	/* Wait for wifi connection */
    	while(Wifi_Ctrl_waitConnection(this->wifiController, WAITING_WIFI_CONNECTION_TIMEOUT) != CUBEE_ERROR_OK)
    	{
    		ESP_LOGI(LOG_HEADER,"Waiting wifi connection");
    		vTaskDelay( MQTT_TASK_DELAY );
    	}

    	/* Set MQTT client ID with ramdom number */
    	randomNumber = READ_PERI_REG(DR_REG_RNG_BASE);
    	snprintf(clientIDBuffer, MAX_CLIENT_ID_SIZE, "CUBEE-%u",randomNumber);
    	this->data.clientID.cstring  = clientIDBuffer;

    	/* Connect MQTT client to MQTT broker */
    	if(MQTTmbedtls_ConnectNetwork(&this->network, this->broker_address, this->broker_port) != SUCCESS)
    	{
			ESP_LOGE(LOG_HEADER, "MQTT Network Connection failed!");
			updateConnection = true;
    	}
    	else if (MQTTClient_Connect(&this->client, &this->data) != SUCCESS) {
			ESP_LOGE(LOG_HEADER, "MQTT client Connection failed!");
			updateConnection = true;
		}
		else if(__MQTT_Network_subscribe(this) != CUBEE_ERROR_OK)	/* Update subscriptions */
		{
			ESP_LOGE(LOG_HEADER, "MQTT Subscriptions failed");
			updateConnection = true;
		}

		xSemaphoreGive(this->semaphore);
    	while(!updateConnection)
    	{
    		yieldFailed = (MQTTClient_Yield(&this->client, 500) != SUCCESS);
    		updateSubscriptionReqest = (xQueueReceive(this->subscriptionUpdate,&updateSubscriptionSignal,MQTT_NETWORK_SIGNAL_QUEUE_TIMEOUT) == pdTRUE);
    		if(yieldFailed || updateSubscriptionReqest)
    		{
    			ESP_LOGE(LOG_HEADER, "MQTT connection needs to be updated, reason: [%d]yield failed; [%d]updateRequest", yieldFailed, updateSubscriptionReqest);
    			updateConnection = true;
    		}
			vTaskDelay( MQTT_TASK_DELAY );
    	}
    	MQTTmbedtls_deinitNetwork(&this->network);
    	this->client.isconnected = false;

		vTaskDelay( MQTT_TASK_DELAY );
    }
 }

static void __MQTT_Network_message_handler(MessageData *md)
{
	MQTT_Network_Data_Packet_t dataPacket;
	char topic[MQTT_NETWORK_TOPIC_SIZE_MAX];

	ESP_LOGI(LOG_HEADER, "Topic received!: %.*s %.*s", md->topicName->lenstring.len, md->topicName->lenstring.data, md->message->payloadlen, (char*)md->message->payload);

	if(mqttNetworkInstance.receiveDataCallback != NULL)
	{
		snprintf(topic, MQTT_NETWORK_TOPIC_SIZE_MAX, "%.*s", md->topicName->lenstring.len, md->topicName->lenstring.data);
		dataPacket.dataBuffer = (uint8_t *) md->message->payload;
		dataPacket.dataBufferSize = (uint8_t) md->message->payloadlen;
		dataPacket.topic = topic;
		mqttNetworkInstance.receiveDataCallback(&dataPacket);
	}
}


static CubeeErrorCode_t __MQTT_Network_subscribe(MQTT_Network_t * this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER on MQTT topics subscriptions");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	for(uint8_t topicIndex = 0; topicIndex < this->subscriptionCount; topicIndex++)
	{
		if((this->subscriptionsTopics[topicIndex] == NULL) || (strlen(this->subscriptionsTopics[topicIndex]) == 0))
		{
			ESP_LOGE(LOG_HEADER, "Subscription failed, invalid topic");
			return CUBEE_ERROR_UNDEFINED;
		}
		else if (MQTTClient_Subscribe(&this->client, this->subscriptionsTopics[topicIndex], QOS1, __MQTT_Network_message_handler) != SUCCESS) {
			ESP_LOGE(LOG_HEADER, "Subscription failed, topic: %s", this->subscriptionsTopics[topicIndex]);
			return CUBEE_ERROR_UNDEFINED;
		}
		else
		{
			ESP_LOGI(LOG_HEADER, "Subscription successful, topic: %s", this->subscriptionsTopics[topicIndex]);
		}
	}
	/* Clean update subscription requests */
	xQueueReset(this->subscriptionUpdate);

	return CUBEE_ERROR_OK;
}
