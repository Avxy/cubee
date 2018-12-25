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

#ifndef WAN_CTRL_H_
#define WAN_CTRL_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

/* Module Dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "WAN_Ctrl_Cfg.h"
#include "../../drv/Wifi_Ctrl/Wifi_Ctrl.h"
#include "../../svc/MQTT/MQTT_Network.h"

#define WAN_CTRL_MAX_DATA_PATH_SIZE			MQTT_NETWORK_TOPIC_SIZE_MAX	/*!< Max size of data paths used to send or receive data over WAN Network */
#define WAN_CTRL_MAX_DATA_SIZE				MQTT_BUF_SIZE				/*!< Max size of data sent or received over WAN Network*/

/************************************************************************************************/
/* WAN Controller TypeDefs */
/************************************************************************************************/

/*!< Struct to define all data types that can be sent over the WAN network */
typedef enum
{
	WAN_CTRL_SND_DATA_COMMAND = 0,
	WAN_CTRL_SND_DATA_ALARM,
	WAN_CTRL_SND_DATA_MEASUREMENT,
	WAN_CTRL_SND_DATA_CONFIG,
	WAN_CTRL_SND_DATA_STATUS,

	WAN_CTRL_SND_DATA_MAX
} WAN_Ctrl_Snd_Data_t;

/*!< Struct to define all data types that can be received over the WAN network */
typedef enum
{
	WAN_CTRL_RCV_DATA_COMMAND = 0,

	WAN_CTRL_RCV_DATA_MAX
} WAN_Ctrl_Rcv_Data_t;

typedef struct
{
	uint8_t dataType;								/*!< Type of data sent or received */
	uint8_t dataBuffer[WAN_CTRL_MAX_DATA_SIZE];		/*!< Stores the data sent or received */
	uint16_t dataBufferSize;						/*!< Size of data sent or received */
} WAN_Ctrl_Data_Packet_t;


/*!< Struct to instantiate a WAN Controller component */
typedef struct
{
	bool initialized;																/*!< Indicates if the WAN Controller component was initialized */
	uint8_t idCubee[ID_CUBEE_MAX_SIZE];												/*!< Defines the CUBEE ID used to communication with the server */
	WAN_Ctrl_Cfg_t * wanControllerCfg;												/*!< Pointer to the configurations used by the WAN Controller instance */
	Wifi_Ctrl_t * wifiController;													/*!< Pointer to wifi controller used by the WAN Controller instance */
	MQTT_Network_t * mqttNetwork;													/*!< Pointer to mqtt network used by the WAN Controller instance */
	QueueHandle_t rcvDataQueue[WAN_CTRL_RCV_DATA_MAX];								/*!< Queues used to store all types of data received over WAN network */
	char sndDataPath[WAN_CTRL_SND_DATA_MAX][WAN_CTRL_MAX_DATA_PATH_SIZE];			/*!< Queues used to store paths like URL, Topic, etc., used to send all types of data over WAN network */
	char rcvDataPath[WAN_CTRL_RCV_DATA_MAX][WAN_CTRL_MAX_DATA_PATH_SIZE];			/*!< Queues used to store paths like URL, Topic, etc., used to receive all types of data over WAN network */

} WAN_Ctrl_t;


/************************************************************************************************/
/* Public API */
/************************************************************************************************/

/**
 * @brief This function initializes the module instance and all its dependencies.
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the module instance initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t WAN_Ctrl_init(WAN_Ctrl_t * this);

/**
* @brief This function gets a module instance.
*
* @return pointer to a module instance
*/
WAN_Ctrl_t * WAN_Ctrl_getInstance(void);

/**
* @brief This function gets the module configuration instance.
*
* @return pointer to module configuration instance.
*/
WAN_Ctrl_Cfg_t * WAN_Ctrl_getCfg(WAN_Ctrl_t* this);

/**
 * @brief This function checks if a module instance is already initialized.
 *
 * @return
 * @arg true, if the module instance is initialized.
 * @arg false, otherwise.
*/
bool WAN_Ctrl_isInitialized(WAN_Ctrl_t * this);

/**
 * @brief This function sends data over WAN network
 *
 * @param this - pointer to module instance.
 * @param dataPacket - packet with information and data to be sent
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the was successfully sent over WAn Network.
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
CubeeErrorCode_t WAN_Ctrl_sendData(WAN_Ctrl_t * this, WAN_Ctrl_Data_Packet_t * dataPacket, bool acknowledgment);

/**
 * @brief This function receives data came from WAN network
 *
 * @param this - pointer to module instance.
 * @param dataPacket - output packet to store information and data received
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully received.
 * @arg CUBEE_ERROR_TIMEOUT, if the WAn Network hasn't data to be received.
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
CubeeErrorCode_t WAN_Ctrl_rcvData(WAN_Ctrl_t * this, WAN_Ctrl_Data_Packet_t * dataPacket);

/**
 * @brief This function updates the path used to send a specific data type over the WAN Network
 *
 * @param this - pointer to module instance.
 * @param sndDataType - send data type
 * @param newPath - New path used to send data
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the send path was successfully updated.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t WAN_Ctrl_updateSndDataPath(WAN_Ctrl_t * this, WAN_Ctrl_Snd_Data_t sndDataType, char * newPath);

/**
 * @brief This function updates the path used to receive a specific data type over the WAN Network
 *
 * @param this - pointer to module instance
 * @param rcvDataType - receive data type
 * @param newPath - New path used to receive data
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the receive path was successfully updated.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t WAN_Ctrl_updateRcvDataPath(WAN_Ctrl_t * this, WAN_Ctrl_Rcv_Data_t rcvDataType, char * newPath);


/**
 * @brief
 *
 * @return
*/
CubeeErrorCode_t WAN_Ctrl_updateCfg(WAN_Ctrl_t * this);

/**
 * @brief
 *
 * @return
*/
CubeeErrorCode_t WAN_Ctrl_getTime(time_t * now);

#endif /* WAN_CTRL_H_ */
