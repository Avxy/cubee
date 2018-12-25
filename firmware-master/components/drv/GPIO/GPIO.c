/*
 * DIO.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "GPIO.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_system.h"

/* Command Queue */
#define BUTTON_QUEUE_LENGTH    					(1)
#define BUTTON_QUEUE_ITEM_SIZE       			sizeof( uint8_t )
#define BUTTON_QUEUE_TIMEOUT					200 / portTICK_PERIOD_MS
#define BUTTON_DEBOUNCE_PERIOD					400 / portTICK_PERIOD_MS
#define ESP_INTR_FLAG_DEFAULT 					(0)

#define LOG_HEADER 								"[GPIO DRV] "

#define 										xDEBUG_GPIO


/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/
/* Command queue */
QueueHandle_t buttonQueueHandle;
uint32_t	buttonDebounceTime = 0;

static GPIO_t gpioInstance =
{
	.initialized = false,
};


/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/
static void __GPIO_isr_handler(void* arg);

/************************************************************************************************/
/* Public API  Implementation */
/************************************************************************************************/

GPIO_Cfg_t * GPIO_getConfig(GPIO_t* this)
{
	return  this->gpioCfg;
}

GPIO_t * GPIO_getInstance()
{
	return  &gpioInstance;
}

CubeeErrorCode_t GPIO_init(GPIO_t* this)
{
	CubeeErrorCode_t cubeeErrorCode;
	esp_err_t espErrorCode;

	if(GPIO_isInitialized(this))
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing GPIO");

	this->gpioCfg = GPIO_Cfg_getInstance();
	cubeeErrorCode = GPIO_Cfg_init(this->gpioCfg);
	if(cubeeErrorCode != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "GPIO_Cfg initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Create the button queue */
	 buttonQueueHandle = xQueueCreate(BUTTON_QUEUE_LENGTH, BUTTON_QUEUE_ITEM_SIZE);

	/* Install gpio isr service */
	espErrorCode = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	if(espErrorCode != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "GPIO ISR service install failed");
		return CUBEE_ERROR_UNDEFINED;
	}
	/* Add handler to button interruptions */
	espErrorCode = gpio_isr_handler_add(GPIO_INPUT_BUTTON, __GPIO_isr_handler, (void*) GPIO_INPUT_BUTTON);
	if(espErrorCode != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "GPIO ISR handler add failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	this->initialized = true;

	ESP_LOGI(LOG_HEADER,"GPIO Initialized");

	return CUBEE_ERROR_OK;
}

bool GPIO_isInitialized(GPIO_t* this)
{
	return  this->initialized;
}

uint32_t GPIO_read(GPIO_t* this, GPIO_ID_t gpioID)
{
	uint32_t level = 0;

	level = gpio_get_level(gpioID);

#ifdef DEBUG_GPIO
	ESP_LOGI(LOG_HEADER, "GPIO %d read, current level: %d", gpioID, level);
#endif /* DEBUG_GPIO */

	return  level;
}

CubeeErrorCode_t GPIO_write(GPIO_t* this, GPIO_ID_t gpioID, uint32_t value)
{
	esp_err_t error;

	error = gpio_set_level(gpioID, value);
	if(error != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error written GPIO %d to level %d", gpioID, value);
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef DEBUG_GPIO
	ESP_LOGI(LOG_HEADER, "GPIO %d written, current level: %d", gpioID, value);
#endif /* DEBUG_GPIO */

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t GPIO_writeByte(GPIO_t* this, GPIO_ID_t * gpioID, uint8_t value, uint8_t size)
{
	CubeeErrorCode_t error = CUBEE_ERROR_OK;

	if((this == NULL) || (gpioID == NULL) || (size > 8))
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER writing multiples GPIOs");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	for (uint8_t i=0; i<size; i++) {
		//ESP_LOGD(LOG_TAG, "i=%d, bits=%d", i, bits);

		if(GPIO_write(this, gpioID[i], (value & (1<<i)) != 0) != ESP_OK)
		{
			error = CUBEE_ERROR_UNDEFINED;
		}
	}

	return error;
}

CubeeErrorCode_t GPIO_readButton(GPIO_t * this, uint8_t * value)
{
	BaseType_t hasCmd;

	if((this = NULL) || (value == NULL))
	{
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	hasCmd = xQueueReceive(buttonQueueHandle,value,BUTTON_QUEUE_TIMEOUT);

	if(hasCmd == pdTRUE)
	{
		ESP_LOGI(LOG_HEADER, "Button signal detected, value: %#02x:", (uint8_t) value[0]);
		return CUBEE_ERROR_OK;
	}

	return CUBEE_ERROR_OK;
}

/************************************************************************************************/
/* Internal Functions Implementation  */
/************************************************************************************************/

static void __GPIO_isr_handler(void* arg)
{
	uint32_t gpio = (uint32_t) arg;
	uint8_t cmd = 0x01;
	uint32_t tick;

	/* Get current system tick*/
	tick = (uint32_t) xTaskGetTickCountFromISR();

	/* Debounce algorithm implementation*/
	if((tick - buttonDebounceTime) >= BUTTON_DEBOUNCE_PERIOD)
	{
		/* Reset debounce time*/
		buttonDebounceTime = tick;
	    switch(gpio)
	    {
	    	case (GPIO_INPUT_BUTTON):
					xQueueOverwriteFromISR(buttonQueueHandle, &cmd, NULL);
	    			break;
	        default:

	            break;
	    }
	}

}

