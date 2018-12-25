/**
 * @file WAN_Ctrl.h
 * @version 2.0
 * @author Alex Fernandes
 * @date October 16, 2017
 **************************************************************************
 *
 * @brief  WAN Controller
 *
 *
 * @section References
 * @ref 1. Hypertext Transfer Protocol -- HTTP/1.0 (https://www.w3.org/Protocols/HTTP/1.0/spec.html)
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   06-Jun-2017    Alex Fernandes
 * * Original version
 *
 *  Revision: 2.0   16-Oct-2017    Alex Fernandes
 * * Architecture change for integration with MQTT service
 ******************************************************************/

/* Module Dependencies */
#include "WAN_Ctrl.h"
#include "WAN_Ctrl_Cfg.h"

/* FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/* ESP Libraries */
#include "esp_log.h"

/* LWIP - TCP/IP Protocol Implementation */
#include "apps/sntp/sntp.h"

#define LOG_HEADER 							"[WAN SVC] "
#define xDEBUG_WAN_CTRL



/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

/**
 * @brief This function initializes the SNTP service
 *
 * @param this - Pointer to module instance
 */
static void __WAN_Ctrl_init_sntp(WAN_Ctrl_t * this);

/**
 * @brief Callback to receive data from MQTT network
 *
 * @param data - data received from MQTT network
 */
static void __WAN_Ctrl_MQTTReceive_calback(MQTT_Network_Data_Packet_t * data);

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/

static WAN_Ctrl_t wanCtrlInstance =
{
		.initialized = false,											/*!< Indicates if the WAN Controller component was initialized */
		.idCubee = "idCubee",											/*!< Defines the CUBEE ID used to communication with the server */
		.wanControllerCfg = NULL,										/*!< Pointer to the configurations used by the WAN Controller instance */
		.wifiController = NULL,											/*!< Pointer to wifi controller used by the WAN Controller instance */
		.mqttNetwork = NULL,											/*!< Pointer to mqtt network used by the WAN Controller instance */
		.rcvDataQueue =						 							/*!< Queues used to store all types of data received over WAN network */
		{
			[WAN_CTRL_RCV_DATA_COMMAND] = NULL,
		},
		.sndDataPath = 													/*!< Queues used to store paths like URL, Topic, etc., used to send all types of data over WAN network */
		{
			[WAN_CTRL_SND_DATA_COMMAND] = "cubee/command",
			[WAN_CTRL_SND_DATA_ALARM] = "cubee/alarm",
			[WAN_CTRL_SND_DATA_MEASUREMENT] = "cubee/measurement",
			[WAN_CTRL_SND_DATA_CONFIG] = "cubee/register",
			[WAN_CTRL_SND_DATA_STATUS] = "cubee/states",
		},
		.rcvDataPath =													/*!< Queues used to store paths like URL, Topic, etc., used to receive all types of data over WAN network */
		{
			[WAN_CTRL_RCV_DATA_COMMAND] = "cubee/0/command",
		},
};

static char timeBuffer[100];

/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t WAN_Ctrl_init(WAN_Ctrl_t * this)
{
	if(this == NULL)
	{
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Verify if the WAN Controller is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing WAN Controller...");

	this->wanControllerCfg = WAN_Ctrl_Cfg_getInstance();
    if (WAN_Ctrl_Cfg_init(this->wanControllerCfg) != CUBEE_ERROR_OK)
    {
        ESP_LOGE(LOG_HEADER, "WAN Controller config initialization failed");
        return CUBEE_ERROR_UNDEFINED;
    }

	this->wifiController = Wifi_Ctrl_getInstance();
	if(Wifi_Ctrl_init(this->wifiController) != CUBEE_ERROR_OK){
		ESP_LOGE(LOG_HEADER, "Wifi controller initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	this->mqttNetwork = MQTT_Network_getInstance();
	MQTT_Network_registerRcvCallback(this->mqttNetwork,__WAN_Ctrl_MQTTReceive_calback);
	/* Create receive data queues */
	for(int rcvDataId = 0; rcvDataId < WAN_CTRL_RCV_DATA_MAX; rcvDataId++)
	{
		this->rcvDataQueue[rcvDataId] = xQueueCreate(WAN_CTRL_DATA_QUEUE_LENGTH, WAN_CTRL_DATA_QUEUE_ITEM_SIZE);
		if(this->rcvDataQueue[rcvDataId] == NULL)
		{
			ESP_LOGE(LOG_HEADER, "Error creating receive data queue for receive data type %d", rcvDataId);
			return CUBEE_ERROR_UNDEFINED;
		}

		/* Add subscription */
		this->mqttNetwork->subscriptionsTopics[rcvDataId] = this->rcvDataPath[WAN_CTRL_RCV_DATA_COMMAND];
		this->mqttNetwork->subscriptionCount++;
		ESP_LOGI(LOG_HEADER, "Data receive path: %s", &this->rcvDataPath[rcvDataId][0]);
	}

	for(int sndDataId = 0; sndDataId < WAN_CTRL_SND_DATA_MAX; sndDataId++)
	{
		ESP_LOGI(LOG_HEADER, "Data send path: %s", &this->sndDataPath[sndDataId][0]);
	}

	if(MQTT_Network_init(this->mqttNetwork) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "MQTT Network initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

    this->initialized = true;

    ESP_LOGI(LOG_HEADER,"WAN Controller initialized");

    return CUBEE_ERROR_OK;
}


WAN_Ctrl_t * WAN_Ctrl_getInstance(void)
{
	return &wanCtrlInstance;
}

WAN_Ctrl_Cfg_t * WAN_Ctrl_getCfg(WAN_Ctrl_t* this)
{
	if(this == NULL)
	{
		return NULL;
	}

	return this->wanControllerCfg;
}

bool WAN_Ctrl_isInitialized(WAN_Ctrl_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

CubeeErrorCode_t WAN_Ctrl_sendData(WAN_Ctrl_t * this, WAN_Ctrl_Data_Packet_t * dataPacket, bool acknowledgment)
{
	MQTT_Network_Data_Packet_t mqttDataPacket;

	if((this == NULL) || (dataPacket == NULL) || (dataPacket->dataBufferSize == 0) || (dataPacket->dataType >= WAN_CTRL_SND_DATA_MAX))
	{
		ESP_LOGI(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER sending data over MQTT network");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(!MQTT_Network_isConnected(this->mqttNetwork))
	{
		/* Network not connected */
		return CUBEE_ERROR_TIMEOUT;
	}

	mqttDataPacket.dataBuffer = dataPacket->dataBuffer;
	mqttDataPacket.dataBufferSize = dataPacket->dataBufferSize;
	mqttDataPacket.topic = this->sndDataPath[dataPacket->dataType];

	if(MQTT_Network_sendData(this->mqttNetwork, &mqttDataPacket, acknowledgment) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error sending data type %d over MQTT network", dataPacket->dataType);
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t WAN_Ctrl_rcvData(WAN_Ctrl_t * this, WAN_Ctrl_Data_Packet_t * dataPacket)
{
	if((this == NULL) || (dataPacket == NULL))
	{
		ESP_LOGI(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER receiving data from MQTT network");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	memset(dataPacket,0,sizeof(WAN_Ctrl_Data_Packet_t));
	/* TODO: remove hard code on data type */
	if(xQueueReceive(this->rcvDataQueue[WAN_CTRL_RCV_DATA_COMMAND], dataPacket, WAN_CTRL_DATA_QUEUE_TIMEOUT) != pdTRUE)
	{
		/* Doesn't has data to be received  */
		return CUBEE_ERROR_TIMEOUT;
	}

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t WAN_Ctrl_updateSndDataPath(WAN_Ctrl_t * this, WAN_Ctrl_Snd_Data_t sndDataType, char * newPath)
{
	if((this == NULL) || (sndDataType >= WAN_CTRL_SND_DATA_MAX) || (newPath == NULL))
	{
		ESP_LOGI(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER updating send data path");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	strcpy(this->sndDataPath[sndDataType], newPath);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t WAN_Ctrl_updateRcvDataPath(WAN_Ctrl_t * this, WAN_Ctrl_Rcv_Data_t rcvDataType, char * newPath)
{
	if((this == NULL) || (rcvDataType >= WAN_CTRL_RCV_DATA_MAX) || (newPath == NULL))
	{
		ESP_LOGI(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER updating receive data path");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	strcpy(this->rcvDataPath[rcvDataType], newPath);
	if(MQTT_Network_updateSubscriptions(this->mqttNetwork) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error sending update subscription signal to MQTT network");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t WAN_Ctrl_updateCfg(WAN_Ctrl_t * this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER updating WAN configuration \n");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	ESP_LOGI(LOG_HEADER, "Updating WAN configuration...");

	/* Update Wifi configuration */
	memset(this->wifiController->wifiControllerCfg->wifiSSID, 0, SSID_MAX_SIZE);
	memset(this->wifiController->wifiControllerCfg->wifiPassword, 0, PASSWORD_MAX_SIZE);
	memmove(this->wifiController->wifiControllerCfg->wifiSSID, this->wanControllerCfg->wifiSSID, SSID_MAX_SIZE);
	memmove(this->wifiController->wifiControllerCfg->wifiPassword, this->wanControllerCfg->wifiPassword, PASSWORD_MAX_SIZE);

	if(Wifi_Ctrl_updateCfg(this->wifiController) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating WAN configuration\n");
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER, "WAN configuration successfully updated");

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t WAN_Ctrl_getTime(time_t * now )
{

    struct tm timeinfo = { 0 };
    uint32_t retry = 0;
    uint32_t retry_count = 5;

    /* Wait for the first synchronization with the SNTP server */
    while(timeinfo.tm_year < (2017 - 1900) && ++retry < retry_count)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        time(now);
        localtime_r(now, &timeinfo);
    }

    if(retry == retry_count)
    {
    	return CUBEE_ERROR_TIMEOUT;
    }

    /* Time was successfully retrieved */
    strftime(timeBuffer, sizeof(timeBuffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
    ESP_LOGI(LOG_HEADER, "Time successfully retrieved: %s", timeBuffer);
    return CUBEE_ERROR_OK;
}

/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/



static void __WAN_Ctrl_init_sntp(WAN_Ctrl_t * this)
{
    ESP_LOGI(LOG_HEADER, "Initializing SNTP");

    sntp_setoperatingmode(SNTP_OPMODE_POLL); /* Periodically query the SNTP servers */
    sntp_setservername(0, "pool.ntp.org"); /* cluster of reliable NTP servers from pool.ntp.org */
    sntp_init();
    setenv("TZ", "CET-1", 1);

    ESP_LOGI(LOG_HEADER, "SNTP successfully initialized");
}

static void __WAN_Ctrl_MQTTReceive_calback(MQTT_Network_Data_Packet_t * data)
{
	WAN_Ctrl_Data_Packet_t wanDataPacket;
	wanDataPacket.dataType = WAN_CTRL_RCV_DATA_MAX;

	if(data->dataBufferSize > WAN_CTRL_MAX_DATA_SIZE)
	{
		ESP_LOGE(LOG_HEADER, "Data received on MQTT network is bigger than WAN max data size");
		return;
	}

	/* Verify data type */
	for(int rcvQeueId = 0; rcvQeueId < WAN_CTRL_RCV_DATA_MAX; rcvQeueId++)
	{
		if(strcmp(wanCtrlInstance.rcvDataPath[rcvQeueId], data->topic) == 0)
		{
			wanDataPacket.dataType = rcvQeueId;
		}
	}

	if(wanDataPacket.dataType < WAN_CTRL_RCV_DATA_MAX)
	{
		memset(wanDataPacket.dataBuffer, 0, WAN_CTRL_MAX_DATA_SIZE);
		wanDataPacket.dataBufferSize = data->dataBufferSize;
		memmove(wanDataPacket.dataBuffer, data->dataBuffer, data->dataBufferSize);

		/* Send the bluetooth signal to the state machine signal queue*/
		if(xQueueOverwrite(wanCtrlInstance.rcvDataQueue[wanDataPacket.dataType], &wanDataPacket) != pdTRUE)
		{
			/* We have finished accessing the shared resource.  Release the	 semaphore. */
			ESP_LOGE(LOG_HEADER,"Error sending receiving data from MQTT Network");
			return;
		}

		ESP_LOGI(LOG_HEADER, "WAN controller received MQTT data type %d on topic %s", wanDataPacket.dataType, wanCtrlInstance.rcvDataPath[wanDataPacket.dataType]);
	}
	else
	{
		ESP_LOGE(LOG_HEADER, "WAN controller received invalid data type %d", wanDataPacket.dataType);
	}
}


