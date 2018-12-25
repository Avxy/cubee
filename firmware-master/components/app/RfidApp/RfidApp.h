/**
 * @file RfidApp.h
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

#ifndef RFID_APP_H_
#define RFID_APP_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/*FreeRTOS libraries*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../util/StateMachine/FSM.h"
#include "RfidApp_Cfg.h"
#include "../../svc/RFID_Reader/RFID_Reader.h"
#include "../../svc/StorageMgr/StorageMgr.h"

/************************************************************************************************/
/* State Machine TypeDefs */
/************************************************************************************************/

/*! Enum with possible states to the RFID state machine */
typedef enum
{
	RFID_APP_NOSTATE = 0,				/*!< No state */
	RFID_APP_STATE_INITIALIZATION,		/*!< Indicates that RFID App is doing initialization task */
	RFID_APP_STATE_IDLE,				/*!< Indicates that RFID is looking for new RFID cards */
	RFID_APP_STATE_CARD_READY,			/*!< Indicates that RFID found a new card */
	RFID_APP_STATE_CARD_ACTIVE,			/*!< Indicates that RFID has select a card */
	RFID_APP_STATE_CARD_VALIDATED,		/*!< Indicates that RFID is communicating with a valid card */
} RfidApp_States_t;

/*! Enum with signals used to stimulate the RFID state machine */
typedef enum
{
	RFID_APP_NOSIG = 0,							/*!< No signal */
	RFID_APP_SIGNAL_INITIALIZED,				/*!< Indicates that initializations task is finished */
	RFID_APP_SIGNAL_COMMUNICATION_FAILURE,		/*!< Indicates that a communication failure has on RFID communication */

	RFID_APP_SIGNAL_MAX,						/*!< Indicates the number of possible signals */
} RfidApp_Signals_t;

/*! Struct with state data to each possible state of the RFID App state machine */
typedef struct
{
    FSM_STATE mainState;
    FSM_STATE Initialization;			/*!< Indicates that RFID App is doing initialization task */
    FSM_STATE Idle;						/*!< Indicates that RFID is looking for new RFID cards */
    FSM_STATE CardReady;				/*!< Indicates that RFID found a new card */
    FSM_STATE CardActive;				/*!< Indicates that RFID has select a card */
    FSM_STATE CardValidated;			/*!< Indicates that RFID is communicating with a valid card */
} RfidApp_STM_t;

/************************************************************************************************/
/* RFID Controller TypeDefs */
/************************************************************************************************/

/**
 * @brief callback used to send instant alarms
 *
 * @param data - pointer to alarm to be sent
 */
typedef CubeeErrorCode_t (*RFIDApp_sendInstantAlarm_callback)(StorageMgr_Alarm_t *alarm);

/*! Struct to instantiate a Rfid app component */
typedef struct
{
	bool initialized;												/*!< Indicates if the RFID application component was initialized */
	RfidApp_Cfg_t * rfidAppCfg;										/*!< Pointer to the configurations used by the RFID instance */
	SemaphoreHandle_t semaphore;									/*!< Semaphore used to synchronize access to RFID app resources */
	QueueHandle_t signalQueueHandle;								/*!< Queue used to store state machine signals and stimulates the RFID app state machine*/
	RfidApp_STM_t stm;												/*!< Pointer to the RFID app state machine data */
	RFID_Reader_t * rfidReader;										/*!< Pointer to the RFID reader instance used on RFID card communication */
	uint8_t readBlock[RFID_APP_BLOCK_SIZE + RFID_APP_CRC_SIZE];		/*!< Buffer used to read card block */
	StorageMgr_t * storageMgr;										/*!< Pointer to the StorageMgr instance used by the RFID APP instance */
	RFIDApp_sendInstantAlarm_callback sendAlarmCallback;			/*!< callback used to send RFID Alarms*/
} RfidApp_t;


/************************************************************************************************/
/* Public API */
/************************************************************************************************/

/**
 * @brief This function initializes a Rfid App component and all its dependencies.
 *
 * @param this - pointer to Rfid App component instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the component Rfid App initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t RfidApp_init(RfidApp_t * this);

/**
* @brief This function gets a Rfid App instance.
*
* @return pointer to a Rfid App component instance
*/
RfidApp_t * RfidApp_getInstance(void);

/**
* @brief This function gets a Rfid App configuration instance.
*
* @return pointer to the Rfid App configuration instance.
*/
RfidApp_Cfg_t * RfidApp_getCfg(RfidApp_t* this);

/**
 * @brief This function checks if a Rfid App instance is already initialized.
 *
 * @return
 * @arg true, if the Rfid App instance is initialized.
 * @arg false, otherwise.
*/
bool RfidApp_isInitialized(RfidApp_t * this);

/**
 * @brief This function sends a signal to the Rfid App state machine
 *
 * @param this - pointer to Rfid App instance.
 * @param sendsignal - signal to be sent
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the signal was successfully sent to the signal queue.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_TIMEOUT, if was not possible access the Rfid App shared resources
 * @arg CUBEE_ERROR_UNDEFINED, if it wasn't possible send the signal cause the signal queue is already full
*/
CubeeErrorCode_t RfidApp_sendSignal(RfidApp_t * this, RfidApp_Signals_t rcvSignal);

/**
 * @brief This function registers the callback used to send RFID alarms.
 *
 * @param this - module instance
 * @param callback - pointer to callback funtion
 *
 * @return
 * @arg CUBEE_ERROR_OK, callback successfully registered.
 * @arg CUBEE_ERROR_CODE, otherwise.
 */
CubeeErrorCode_t RfidApp_registerRcvCallback(RfidApp_t * this, RFIDApp_sendInstantAlarm_callback callback);


#endif /* RFID_APP_H_ */
