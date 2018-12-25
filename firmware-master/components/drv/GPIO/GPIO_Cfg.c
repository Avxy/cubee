/*
 * DIO_Cfg.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */


/* ESP Libraries */
#include "esp_log.h"
#include "esp_err.h"

/* Module Dependencies */
#include "GPIO_Cfg.h"

#define LOG_HEADER 		"[GPIO CFG DRV] "

static GPIO_Cfg_t gpioCfgInstance =
{
		.initialized = false,
		.configTable =
				{
						[GPIO_CONFIG_ID_RELE] =
						{

							    .pin_bit_mask = (1<<GPIO_OUTPUT_RELE),       /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
							    .mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
							    .pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
							    .pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
							    .intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */

						},

						[GPIO_CONFIG_ID_LED] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_LED),        /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},

						[GPIO_CONFIG_ID_BUTTON] =
						{

								.pin_bit_mask = (1<<GPIO_INPUT_BUTTON),        /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_INPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_NEGEDGE                    /*!< GPIO interrupt type                                  */
						},

#ifdef USING_DB9
						[GPIO_CONFIG_DB9_0] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_DB9_0),		      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},

						[GPIO_CONFIG_DB9_1] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_DB9_1),		      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},

						[GPIO_CONFIG_DB9_2] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_DB9_2),		      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},

						[GPIO_CONFIG_DB9_3] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_DB9_3),		      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},

						[GPIO_CONFIG_DB9_4] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_DB9_4),		      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},

						[GPIO_CONFIG_DB9_5] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_DB9_5),		      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},

						[GPIO_CONFIG_DB9_6] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_DB9_6),		      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},

						[GPIO_CONFIG_DB9_7] =
						{

								.pin_bit_mask = (1<<GPIO_OUTPUT_DB9_7),		      /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
								.mode = GPIO_MODE_OUTPUT,                         /*!< GPIO mode: set input/output mode                     */
								.pull_up_en = GPIO_PULLUP_ENABLE,                 /*!< GPIO pull-up                                         */
								.pull_down_en = GPIO_PULLDOWN_DISABLE,            /*!< GPIO pull-down                                       */
								.intr_type = GPIO_INTR_DISABLE                    /*!< GPIO interrupt type                                  */
						},
#endif /* USING_DB9 */
				},
};


GPIO_Cfg_t * GPIO_Cfg_getInstance()
{
	return  &gpioCfgInstance;
}

CubeeErrorCode_t GPIO_Cfg_init(GPIO_Cfg_t* this)
{
	CubeeErrorCode_t error = CUBEE_ERROR_OK;

	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER initializing GPIO configuration");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Verify if the RFID APP is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	for(uint8_t gpioIndex=0 ; gpioIndex < GPIO_CONFIG_ID_MAX ; gpioIndex++)
	{
		if(gpio_config(&this->configTable[gpioIndex]) != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER, "GPIO PIN initialization failed, GPIO Index %d", gpioIndex);
			error = CUBEE_ERROR_UNDEFINED;
		}
	}

	this->initialized = true;

	return  error;
}
