/*
 * ADC_Cfg.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef _ADC_CFG_H_
#define _ADC_ADC_CFG_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "driver/adc.h"

#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define ADC_CHANNEL_CURRENT			ADC1_CHANNEL_7
#define ADC1_BIT_WIDTH				ADC_WIDTH_12Bit

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	ADC_CURRENT_SENSOR_CFG_ID = 0,

	ADC_CONFIG_ID_MAX
} ADC_Config_ID_t;

typedef struct
{
	adc1_channel_t channel;
	adc_bits_width_t bitsWidth;
	adc_atten_t 	attenuation;

} ADC_config_parameters_t;

typedef struct ADC_Cfg
{
	bool initialized;
	ADC_config_parameters_t configTable[ADC_CONFIG_ID_MAX];
} ADC_Cfg_t;

ADC_Cfg_t * ADC_Cfg_getInstance();
CubeeErrorCode_t ADC_Cfg_init(ADC_Cfg_t* this);

#ifdef __cplusplus
}
#endif

#endif /* _ADC_CFG_H_ */
