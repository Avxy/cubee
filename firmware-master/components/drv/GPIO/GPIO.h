/*
 * DIO.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef DIO_H_
#define DIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <string.h>

#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../../main/ProjectCfg.h"
#include "GPIO_Cfg.h"

#define PIN_ON			(1)
#define PIN_OFF			(0)

/*! Enum to index the GPIOs */
typedef enum
{
	GPIO_RELE = GPIO_OUTPUT_RELE,
	GPIO_LED = GPIO_OUTPUT_LED,
	GPIO_BUTTON = GPIO_INPUT_BUTTON,

#ifdef USING_DB9
	GPIO_DB9_0 = GPIO_OUTPUT_DB9_0,
	GPIO_DB9_1 = GPIO_OUTPUT_DB9_1,
	GPIO_DB9_2 = GPIO_OUTPUT_DB9_2,
	GPIO_DB9_3 = GPIO_OUTPUT_DB9_3,
	GPIO_DB9_4 = GPIO_OUTPUT_DB9_4,
	GPIO_DB9_5 = GPIO_OUTPUT_DB9_5,
	GPIO_DB9_6 = GPIO_OUTPUT_DB9_6,
	GPIO_DB9_7 = GPIO_OUTPUT_DB9_7,
#endif /* USING_DB9 */

} GPIO_ID_t;

/*! Struct to instantiate the GPIO component */
typedef struct GPIO
{
	bool initialized;						/*!< Indicates if the GPIO component was initialized */

	GPIO_Cfg_t *gpioCfg;						/*!< Pointer to the configurations used by the GPIO instance */
} GPIO_t;

/**
 * @brief This function initializes the GPIO instance and all its dependencies.
 *
 * @param this - pointer to GPIO component instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the component GPIO initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t GPIO_init(GPIO_t* this);

/**
* @brief This function gets the GPIO configuration instance.
*
* @return pointer to the GPIO configuration instance.
*/
GPIO_Cfg_t * GPIO_getConfig(GPIO_t* this);

/**
* @brief This function gets the GPIO instance.
*
* @return pointer to GPIO component instance.
*/
GPIO_t * GPIO_getInstance();

/**
 * @brief This function checks if the GPIO instance is already initialized.
 *
 * @return
 * @arg true, if the GPIO instance is initialized.
 * @arg false, otherwise.
*/
bool GPIO_isInitialized(GPIO_t* this);

/**
 * @brief Reads the current state of a GPIO.
 *
 * @param this - pointer to GPIO component instance.
 * @param gpioID - GPIO identification.
 *
 * @return
*/
uint32_t GPIO_read(GPIO_t* this, GPIO_ID_t gpioID);

/**
 * @brief Sets the state of a GPIO.
 *
 * @param this - pointer to GPIO component instance.
 * @param gpioID - GPIO identification.
 * @param value - new state.
 *
 * @return
*/
CubeeErrorCode_t GPIO_write(GPIO_t* this, GPIO_ID_t gpioID, uint32_t value);

/**
 * @brief Defines the state of a set of GPIO in output mode, up to 8 GPIOs
 *
 * @param this - pointer to GPIO component instance.
 * @param gpioID - set of GPIO identifiers.
 * @param value - byte defining the state of each GPIO: bit0 -> gpioID[0] ... bit7 -> gpioID[7].
 * @param size - Number of GPIOs, up to 8 GPIOs.
 *
 * @return
*/
CubeeErrorCode_t GPIO_writeByte(GPIO_t* this, GPIO_ID_t * gpioID, uint8_t value, uint8_t size);

/**
 * @brief Reads the last button event using software debounce
 *
 * @param this - pointer to the GPIO component instance.
 * @param value - output parameter to the current button
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the button event reading was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t GPIO_readButton(GPIO_t * this, uint8_t * value);



#ifdef __cplusplus
}
#endif

#endif /* DIO_H_ */
