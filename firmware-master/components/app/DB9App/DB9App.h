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
 ****************************************************************************/

#ifndef DB9_APP_H_
#define DB9_APP_H_

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
#include "DB9App_Cfg.h"
#include "../../svc/IOMgr/IOMgr.h"
#include "../../svc/StorageMgr/StorageMgr.h"

/************************************************************************************************/
/* State Machine TypeDefs */
/************************************************************************************************/

/*! Enum with possible states to the DB9 App state machine */
typedef enum
{
	DB9_APP_NOSTATE = 0,				/*!< No state */
	DB9_APP_STATE_INITIALIZATION,		/*!< Indicates that DB9 App is doing initialization task */
	DB9_APP_STATE_IDLE,					/*!< Indicates that DB9 is preparing to initiate a rule execution. */
	DB9_APP_STATE_DEACTIVATED,			/*!< The rule execution is deactivated and the DB9 App can execute DB9 commands */
	DB9_APP_STATE_RUNNING_RULE,			/*!< Indicates that a DB9 rule is being executed */
} DB9App_States_t;

/*! Enum with signals used to stimulate the DB9 App state machine */
typedef enum
{
	DB9_APP_NOSIG = 0,					/*!< No signal */
	DB9_APP_SIGNAL_INITIALIZED,			/*!< Indicates that initializations task is finished */
	DB9_APP_SIGNAL_COMMAND,				/*!< Signal used to execute the DB9 command configured in the DB9App_t instance */

	DB9_APP_SIGNAL_MAX,					/*!< Indicates the number of possible signals */
} DB9App_Signals_t;

/*! Struct with state data to each possible state of the DB9 App state machine */
typedef struct
{
    FSM_STATE mainState;
    FSM_STATE Initialization;			/*!< Indicates that DB9 App is doing initialization task */
    FSM_STATE Idle;						/*!< Indicates that DB9 is preparing to initiate a rule execution. */
    FSM_STATE Deactivated;				/*!< The rule execution is deactivated and the DB9 App can execute DB9 commands */
    FSM_STATE RunninRule;				/*!< Indicates that a DB9 rule is being executed */
} DB9App_STM_t;

/************************************************************************************************/
/* Module Specific TypeDefs */
/************************************************************************************************/

/*! Struct that define a DB9 rule state*/
typedef struct
{
	uint32_t timeDuration;				/*!< Duration in seconds that the state shall be executed */
	uint8_t value;						/*!< Each bit of value indicate the state of a respective DB9 output */
}DB9App_ruleState_t;

/*! Struct that define a DB9 rule */
typedef struct
{
	uint8_t numberOfStates;								/*!< Number of rule states */
	DB9App_ruleState_t ruleStates[DB9_RULE_STATE_MAX];	/*!< List of states composing the rule */
} DB9App_rule_t;

/*! Struct to instantiate the module */
typedef struct
{
	bool initialized;										/*!< Indicates if the module was initialized */
	DB9App_Cfg_t * db9AppCfg;								/*!< Pointer to the configurations used by the module instance */
	SemaphoreHandle_t semaphore;							/*!< Semaphore used to synchronize access to module resources */
	QueueHandle_t signalQueueHandle;						/*!< Queue used to store state machine signals and stimulates the module state machine*/
	DB9App_STM_t stm;										/*!< Pointer to module state machine data */
	IOMgr_t * ioMgr;										/*!< Pointer to the IO Mgr instance used by the DB9 App module */
	DB9App_rule_t currentRule;								/*!< DB9 rule in execution */
	QueueHandle_t newRuleQueueHandler;						/*!< Queue used to store a new rule and update the rule in execution */
	DB9App_rule_t newRule;									/*!< Buffer to a new rule */
	DB9App_ruleState_t db9Command;							/*!< DB9 state used in the execution of a DB9 command */
	bool ruleExecutionEnable;								/*!< Indicates if the rule execution mechanism is enabled or disabled */
	int8_t stateExecutionCounter;							/*!< Indicates the rule state in execution, -1 indicate none rule state in execution */
	bool hasDB9Rule;										/*!< Indicates the module has a rule to be executed */
	StorageMgr_t * storageMgr;								/*!< Pointer to storageMgr instance used by the DB9 App module */
} DB9App_t;


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
CubeeErrorCode_t DB9App_init(DB9App_t * this);

/**
* @brief This function provides a module instance.
*
* @return pointer to a module instance
*/
DB9App_t * DB9App_getInstance(void);

/**
* @brief This function gets the module configuration.
*
* @return pointer to the module configuration.
*/
DB9App_Cfg_t * DB9App_getCfg(DB9App_t* this);

/**
 * @brief This function checks if the module instance is already initialized.
 *
 * @return
 * @arg true, if the module instance is initialized.
 * @arg false, otherwise.
*/
bool DB9App_isInitialized(DB9App_t * this);

/**
 * @brief This function sends a signal to the module state machine
 *
 * @param this - pointer to module instance.
 * @param sendsignal - signal to be sent
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the signal was successfully sent to the signal queue.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_TIMEOUT, if was not possible access the module shared resources
 * @arg CUBEE_ERROR_UNDEFINED, if it wasn't possible send the signal cause the signal queue is already full
*/
CubeeErrorCode_t DB9App_sendSignal(DB9App_t * this, DB9App_Signals_t rcvSignal);

/**
 * @brief This function sends a new rule to the DB9  queue of the DB9App_t instance.
 * The execution of the new rule will begins when the execution of the current rule finish.
 * Furthermore, the rule received will be saved in flash memory.
 *
 * @param this - pointer to module instance.
 * @param rule - new rule to be placed in the rule queue.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the new DB9 rule was successfully sent to the DB9 rule queue.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_TIMEOUT, if was not possible access the module shared resources
 * @arg CUBEE_ERROR_UNDEFINED, otherwise
*/
CubeeErrorCode_t DB9App_sendDB9Rule(DB9App_t * this, DB9App_rule_t * rule);

/**
 * @brief This function enable or disable DB9 rule execution engine.
 *
 * If the rule execution is going to be disabled, the execution of the current
 * rule will be fished at first.
 *
 * @param this - pointer to module instance.
 * @param ruleExecutionEnable - true, enable rule execution
 * 								false, disable rule execution
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the rule execution was successfully enabled or disabled
 * @arg CUBEE_ERROR_TIMEOUT, if was not possible access the module shared resources
 * @arg CUBEE_ERROR_UNDEFINED, otherwise
*/
CubeeErrorCode_t DB9App_setDB9RuleExecution(DB9App_t * this, bool ruleExecutionEnable);

#endif /* DB9_APP_H_ */
