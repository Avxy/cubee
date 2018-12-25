/**
 * @file DB9App_Cfg.h
 * @version 1.0
 * @author Alex Fernandes
 * @date September 29, 2017
 **************************************************************************
 *
 * @brief  DB9 App configuration
 * This module has configurations used by the DB9 application.
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   29-Sep-2017    Alex Fernandes
 * * Original version
 ******************************************************************/

#ifndef DB9_APP_CFG_H_
#define DB9_APP_CFG_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* Modules dependencies */
#include "../../../main/ProjectCfg.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define DB9_APP_SIGNAL_QUEUE_LENGTH			(5)										/*!< Length of queue used to store module state machine signals */
#define DB9_APP_SIGNAL_QUEUE_ITEM_SIZE  	sizeof(DB9App_Signals_t )				/*!< Size of module state machine signals */
#define DB9_APP_SIGNAL_QUEUE_TIMEOUT		(1 / portTICK_PERIOD_MS)				/*!< Waiting timeout to receive a signal from the module state machine queue */
#define DB9_APP_RULE_QUEUE_LENGTH			(1)										/*!< Length of queue used to store a new DB9 rule */
#define DB9_APP_RULE_QUEUE_ITEM_SIZE   		sizeof(DB9App_rule_t)					/*!< Size of DB9 rule */
#define DB9_APP_RULE_QUEUE_TIMEOUT			(1 / portTICK_PERIOD_MS)				/*!< Waiting timeout to receive a rule from the DB9 rule queue */
#define DB9_APP_SEMAPHORE_TIMEOUT			1000 / portTICK_PERIOD_MS				/*!< Waiting timeout to access module shared resources */


/*! Struct to instantiate a module configuration */
typedef struct
{
	bool initialized;	/*!< Indicates if the module configuration was initialized */

} DB9App_Cfg_t;

/**
 * @brief This function initializes a module configuration and all its dependencies.
 *
 * @param this - pointer to module configuration instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK if the module configuration was successfully initialized.
 * @arg CUBEE_ERROR_CODE otherwise.
*/
CubeeErrorCode_t DB9App_Cfg_init(DB9App_Cfg_t * this);

/**
 * @brief This function gives a instance of module configuration.
 *
 * @return pointer to instance of module configuration
*/
DB9App_Cfg_t* DB9App_Cfg_getInstance(void);

/**
 * @brief This function checks if the module configuration is already initialized.
 *
 * @return
 * @arg true, if the module configuration is initialized.
 * @arg false, otherwise.
*/
bool DB9App_Cfg_isInitialized(DB9App_Cfg_t * this);

#endif /* DB9_APP_CFG_H_ */
 
