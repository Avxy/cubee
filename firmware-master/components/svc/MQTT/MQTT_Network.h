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

#ifndef SVC_MQTT_NETWORK_H_
#define SVC_MQTT_NETWORK_H_

/* Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "MQTTClient-C/src/MQTTClient.h"
#include "../../drv/Wifi_Ctrl/Wifi_Ctrl.h"

#define MQTT_NETWORK_SUBSCRIPTIONS_MAX	MAX_MESSAGE_HANDLERS
#define MQTT_NETWORK_TOPIC_SIZE_MAX		(256)
#define MQTT_BUF_SIZE 1024

/************************************************************************************************/
/* Module Specific TypeDefs */
/************************************************************************************************/

typedef struct
{
	char * topic;					/*!< Topic of data sent or received */
	uint8_t * dataBuffer;			/*!< Stores the data sent or received */
	uint16_t dataBufferSize;		/*!< Size of data sent or received */
} MQTT_Network_Data_Packet_t;

/**
 * @brief callback used to notify about data received over MQTT network
 *
 * @param data - pointer to buffer with information and data received
 */
typedef void (* mqtt_rcv_cb_t)(MQTT_Network_Data_Packet_t * data);

/*! Struct to instantiate the module */
typedef struct
{
	bool initialized;												/*!< Indicates if the module instance was initialized */
	Wifi_Ctrl_t * wifiController;									/*!< Pointer to wifi controller used by the module instance */
	QueueHandle_t subscriptionUpdate;								/*!< Queues used to store requests for subscriptions update */
	char * subscriptionsTopics[MQTT_NETWORK_SUBSCRIPTIONS_MAX];		/*!< Topics used in subscriptions */
	uint8_t subscriptionCount;										/*!< Counter for number of subscriptions */
	mqtt_rcv_cb_t receiveDataCallback;								/*!< callback used to notify about data received over MQTT network */
	MQTTClient client;												/*!< client MQTT */
	MQTTmbedtls_Network network;												/*!< Network used to connect the MWTT client */
	MQTTPacket_connectData data;									/*!< Settings to MQTT client connection  */
	char * broker_address;											/*!< Address to MQTT Broker connection  */
	uint16_t broker_port;											/*!< Port to MQTT Broker connection  */
	MQTTString clientId;
	SemaphoreHandle_t semaphore;									/*!< Semaphore used to synchronize access to MQTT Network shared resources */
} MQTT_Network_t;

/************************************************************************************************/
/* Public API */
/************************************************************************************************/

/**
 * @brief This function initializes the module and all its dependencies.
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the module initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t MQTT_Network_init(MQTT_Network_t * mqttNetwork);

/**
* @brief This function provides a module instance.
*
* @return pointer to a module instance
*/
MQTT_Network_t * MQTT_Network_getInstance(void);

/**
 * @brief This function sends a data to a specific topic over the MQTT connection.
 *
 * @param this - module instance
 * @param data - pointer to buffer with information and data to be sent
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully sent.
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
CubeeErrorCode_t MQTT_Network_sendData(MQTT_Network_t * this, MQTT_Network_Data_Packet_t * data, bool acknowledgment);

/**
 * @brief This function signalizes a request for subscriptions updates.
 *
 * The MQTT Network unsubscribe all current topics and subscribes to the topics defined
 * at subscriptionsTopics field in module instance.
 *
 * @param this - module instance
 *
 * @return
 * @arg CUBEE_ERROR_OK, subscription update was successfully signalized.
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
CubeeErrorCode_t MQTT_Network_updateSubscriptions(MQTT_Network_t * this);

/**
 * @brief This function signalizes a request for subscriptions updates.
 *
 * The MQTT Network unsubscribe all current topics and subscribes to the topics defined
 * at subscriptionsTopics field in module instance.
 *
 * @param this - module instance
 *
 * @return
 * @arg CUBEE_ERROR_OK, subscription update was successfully signalized.
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
CubeeErrorCode_t MQTT_Network_updateSubscriptions(MQTT_Network_t * this);

/**
 * @brief This function registers the callback used to notify about data received over MQTT network.
 *
 * @param this - module instance
 * @param callback - pointer to callback funtion
 *
 * @return
 * @arg CUBEE_ERROR_OK, callback successfully registered.
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
CubeeErrorCode_t MQTT_Network_registerRcvCallback(MQTT_Network_t * this, mqtt_rcv_cb_t callback);

/**
 * @brief Verifies if the MQTT network is currently connected
 *
 * @param this - module instance
 *
 * @return
 * @arg true, if the MQTT network is connected
 * @arg false, otherwise
 */
bool MQTT_Network_isConnected(MQTT_Network_t * this);


#endif /* SVC_MQTT_NETWORK_H_ */
