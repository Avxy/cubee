/*
 * ADC.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef ADC_H_
#define ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <string.h>

#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "ADC_Cfg.h"

/*! Enum to index the ADC channels */
typedef enum
{
	ADC_CURRENT_SENSOR_ID = ADC_CHANNEL_CURRENT,
} ADC_ID_t;


/*! Struct to instantiate the ADC component */
typedef struct ADC
{
	bool initialized;						/*!< Indicates if the ADC component was initialized */

	ADC_Cfg_t *adcCfg;						/*!< Pointer to the configurations used by the ADC instance */
} ADC_t;

/**
 * @brief This function initializes the ADC instance and all its dependencies.
 *
 * @param this - pointer to ADC component instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the component ADC initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t ADC_init(ADC_t* this);

/**
* @brief This function gets the ADC configuration instance.
*
* @return pointer to the ADC configuration instance.
*/
ADC_Cfg_t * ADC_getConfig(ADC_t* this);

/**
* @brief This function gets the ADC instance.
*
* @return pointer to ADC component instance.
*/
ADC_t * ADC_getInstance();

/**
 * @brief This function checks if the ADC instance is already initialized.
 *
 * @return
 * @arg true, if the ADC instance is initialized.
 * @arg false, otherwise.
*/
bool ADC_isInitialized(ADC_t* this);

/**
 * @brief Reads the current value of an ADC channel.
 *
 * @param this - pointer to ADC component instance.
 * @param gpioID - ADC channel identification.
 *
 * @return
*/
int32_t ADC_read(ADC_t* this, ADC_ID_t adcID);



#ifdef __cplusplus
}
#endif

#endif /* ADC_H_ */
