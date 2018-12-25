/*
 * ADC_Cfg.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "ADC_Cfg.h"


static ADC_Cfg_t adcCfgInstance =
{
		.initialized = false,
		.configTable =
				{
						[ADC_CURRENT_SENSOR_CFG_ID] =
						{

							    .channel = ADC_CHANNEL_CURRENT,
							    .bitsWidth = ADC1_BIT_WIDTH,
							    .attenuation = ADC_ATTEN_11db,

						},
				},
};

ADC_Cfg_t * ADC_Cfg_getInstance()
{
	return  &adcCfgInstance;
}

CubeeErrorCode_t ADC_Cfg_init(ADC_Cfg_t* this)
{
	this->initialized = true;

	return  CUBEE_ERROR_OK;
}
