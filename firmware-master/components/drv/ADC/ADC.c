/*
 * DIO.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "ADC.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/adc.h"

#define LOG_HEADER 								"[ADC DRV] "

#define 										DEBUG_ADC


/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/

static ADC_t adcInstance =
{
	.initialized = false,
};


/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/

/************************************************************************************************/
/* Public API  Implementation */
/************************************************************************************************/

ADC_Cfg_t * ADC_getConfig(ADC_t* this)
{
	return  this->adcCfg;
}

ADC_t * ADC_getInstance()
{
	return  &adcInstance;
}

CubeeErrorCode_t ADC_init(ADC_t* this)
{
	CubeeErrorCode_t cubeeErrorCode;
	uint8_t adcId;

	if(ADC_isInitialized(this))
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing ADC .. ");

	this->adcCfg = ADC_Cfg_getInstance();
	cubeeErrorCode = ADC_Cfg_init(this->adcCfg);
	if(cubeeErrorCode != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "ADC config initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	if(adc1_config_width(ADC1_BIT_WIDTH) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "ADC1 channel initialization failed, Channel Id");
		return CUBEE_ERROR_UNDEFINED;
	}

	for(adcId=0 ; adcId < ADC_CONFIG_ID_MAX ; adcId++)
	{
		if(adc1_config_channel_atten(this->adcCfg->configTable[adcId].channel, this->adcCfg->configTable[adcId].attenuation) != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER, "ADC channel initialization failed, Channel Id %d", adcId);
			return CUBEE_ERROR_UNDEFINED;
		}
	}

	this->initialized = true;

	ESP_LOGI(LOG_HEADER,"ADC Initialized");

	return CUBEE_ERROR_OK;
}

bool ADC_isInitialized(ADC_t* this)
{
	return  this->initialized;
}

/**
 * @brief Reads the current value of an ADC channel.
 *
 * @param this - pointer to ADC component instance.
 * @param gpioID - ADC channel identification.
 *
 * @return
*/
int32_t ADC_read(ADC_t* this, ADC_ID_t adcID)
{
	int32_t voltageValue = -1;

	if((this == NULL) || ((uint16_t)adcID >= (uint16_t)ADC1_CHANNEL_MAX))
	{
		ESP_LOGE(LOG_HEADER,"ERROR_INVALID_PARAMETER  reading ADC voltage");
		return -1;
	}

	voltageValue = adc1_get_voltage(adcID);
	if(voltageValue == -1)
	{
		ESP_LOGE(LOG_HEADER,"Error  reading ADC voltage, channel %d", adcID);
		return -1;
	}

	return voltageValue;
}

/************************************************************************************************/
/* Internal Functions Implementation  */
/************************************************************************************************/


