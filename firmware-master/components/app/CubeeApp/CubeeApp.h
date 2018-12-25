/**
 * @file CubeeApp.h
 * @version 1.0
 * @author Alex Fernandes
 * @date June 01, 2017
 **************************************************************************
 *
 * @brief  CubeeApp service
 *
 ** The CubeeApp is the application responsible for managing the sending of CUBEE measurements,
 ** alarms and status information to the WEB Server and mobile application. Furthermore, the CubeeApp
 ** processes commands from the PAN and WAN network.
 *
 * @section References
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   01-Jun-2017    Alex Fernandes
 * * Original version
 ******************************************************************/


#ifndef CUBEE_APP_H_
#define CUBEE_APP_H_

/*Standard libraries*/
#include "CubeeApp_Cfg.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "../../../main/ProjectCfg.h"

/* Modules dependencies */
#include "../../util/StateMachine/FSM.h"
#include "../../svc/WANController/WAN_Ctrl.h"
#include "../../svc/PANController/PANController.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../svc/StorageMgr/StorageMgr.h"
#include "../../svc/IOMgr/IOMgr.h"
#ifdef USING_SENSOR_MGR
#include "../../svc/SensorMgr/SensorMgr.h"
#endif /* USING_SENSOR_MGR */
#ifdef USING_RFID
#include "../../app/RfidApp/RfidApp.h"
#endif /* USING_RFID */
#ifdef USING_DB9
#include "../../app/DB9App/DB9App.h"
#endif /* USING_DB9 */

/************************************************************************************************/
/* State Machine TypeDefs */
/************************************************************************************************/

/*! Enum with possible states to the CubeeApp state machine */
typedef enum
{
    CubeeApp__NOSTATE = 0,
	CubeeApp__Initialization,
	CubeeApp__Configuration,
    CubeeApp__Idle,
	CubeeApp__SendingNotification
} CubeeApp_States_t;

/*! Struct with state data to each possible state to the CubeeApp state machine */
typedef struct CubeeApp_STMStruct
{
    FSM_STATE mainState;
    FSM_STATE Configuration;
    FSM_STATE Idle;
    FSM_STATE Initialization;
    FSM_STATE SendingNotification;
} CubeeApp_STM_t;

/*! Enum with signals used to stimulate the CubeeApp state machine */
typedef enum
{
    NOSIG = 0,			/*!< No signal */
} CubeeApp_Signals_t;


/************************************************************************************************/
/* CubeeApp TypeDefs */
/************************************************************************************************/

/*! Enum with commands that can be received by the CUBEE over the PAN or WAN network  */
typedef enum
{
	CUBEE_APP_RCV_CMD_NONE = 0x00,			/*!< No command */
	CUBEE_APP_RCV_CMD_ACTIVATE,				/*!< Activate CUBEE */
	CUBEE_APP_RCV_CMD_DEACTIVATE,			/*!< Deactivate CUBEE */
	CUBEE_APP_RCV_CMD_LED,					/*!< Switch CUBEE LED state */
	CUBEE_APP_RCV_CMD_CONFIG,				/*!< Switch CUBEE LED state */
	CUBEE_APP_RCV_CMD_DB9_RULE,				/*!< Update DB9 rule */
	CUBEE_APP_RCV_CMD_DB9,					/*!< DB9 command */
	CUBEE_APP_RCV_CMD_REG_ACK,				/*!< Acknowledge for CUBEE registration */

	CUBEE_APP_RCV_CMD_MAX,					/*!< Amount of existing commands */
}CubeeApp_RcvCommands_t;

/*! Enum with commands that can be sent by the CUBEE over the PAN or WAN network  */
typedef enum
{
	CUBEE_APP_SND_CMD_NONE = 0x00,			/*!< No command */
	CUBEE_APP_SND_CMD_BUTTON = 0x01,		/*!< Indicates the CUBEE button was pressed */

	CUBEE_APP_SND_CMD_MAX,					/*!< Amount of existing commands */
}CubeeApp_SndCommands_t;

/*! Enum with possible CUBBE PAn network mode  */
typedef enum
{
	CUBEE_APP_MODE_PUBLIC = 0,				/*!< Mode public, CUBEE is visible on the PAN network */
	CUBEE_APP_MODE_PRIVATE,					/*!< Mode private, CUBEE isn't visible on the PAN network */

	CUBEE_APP_MODE_MAX,						/*!< Amount of existing network modes */
}CubeeApp_Mode_t;

/*! Struct to instantiate a CubeeApp component */
typedef struct
{
	bool initialized;						/*!< Indicates if the CubeeApp instance is initialized */
	bool cubeeState;						/*!< Keeps the current cubee activation status: true - CUBEE Activated; false - CUBEE Deactivated */
	bool sendState;							/*!< Indicates if the CubeeApp shall send state over network */

	IOMgr_t *ioMgr;							/*!< Pointer to the IO Mgr */
	CubeeApp_Cfg_t * cubeeAppCfg;			/*!< Pointer to the configurations used by the CubeeApp instance */
	PANController * panControler;			/*!< Pointer to the PAN Controller instance used by the CubeeApp instance */
	WAN_Ctrl_t * wanControler;				/*!< Pointer to the WAN Controller instance used by the CubeeApp instance */
	StorageMgr_t * storageMgr;				/*!< Pointer to the StorageMgr instance used by the CubeeApp instance */
#ifdef USING_SENSOR_MGR
	SensorMgr_SensorId_t sensorId;			/*!< Id of sensor used by the application */
	SensorMgr_t * sensorMgr;				/*!< Pointer to the PowerSensor instance used by the CubeeApp instance */
#endif /* USING_SENSOR_MGR */
#ifdef USING_RFID
	RfidApp_t * rfidAPP;					/*!< Pointer to the RFID APP instance used by the CubeeApp instance */
#endif /* USING_RFID */
#ifdef USING_DB9
	DB9App_t * db9App;						/*!< Pointer to the DB9 APP instance used by the CubeeApp instance */
#endif /* USING_DB9 */

	CubeeApp_STM_t stm;						/*!< Pointer to the CubeeApp state machine data */
} CubeeApp_t;

/************************************************************************************************/
/* Public API */
/************************************************************************************************/

/**
 * @brief This function initializes a CubeeApp instance and all its dependencies.
 *
 * @param this - pointer to the CubeeApp instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the CubeeApp instance initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t CubeeApp_init(CubeeApp_t* this);

/**
* @brief This function gets a CubeeApp instance.
*
* @return pointer to the CubeeApp instance
*/
CubeeApp_t* CubeeApp_getInstance(void);

/**
* @brief This function gets a CubeeApp configuration instance.
*
* @return pointer to the CubeeApp configuration instance
*/
CubeeApp_Cfg_t * CubeeApp_getCfg(CubeeApp_t* this);

/**
 * @brief This function checks if a CubeeApp instance is already initialized.
 *
 * @return
 * @arg true, if the CubeeApp instance is initialized.
 * @arg false, otherwise.
*/
bool CubeeApp_isInitialized(CubeeApp_t * this);

/**
 * @brief This function is used to add an instant alarm to the respective queue,
 * this alarm will be sent as soon as possible by the CUBEE app application.
 *
 * This function may be used as callback to application modules that need send instant alarms to the server.
 *
 * @param alarm - Pointer to alarm to be sent
 *
 * @return
 * @param CUBEE_ERROR_OK - if the alarm was successfully add to the queue.
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
CubeeErrorCode_t CubeeApp_sendInstantAlarm(StorageMgr_Alarm_t *alarm);


#endif /* CUBEE_APP_H_ */
