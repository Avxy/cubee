/*
 * DIO_Cfg.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef COMPONENTS_DRV_DIO_DIO_CFG_H_
#define COMPONENTS_DRV_DIO_DIO_CFG_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "../../../main/ProjectCfg.h"

#ifdef USING_RFID
#define	GPIO_INPUT_BUTTON	(GPIO_NUM_0)
#define GPIO_OUTPUT_RELE    (GPIO_NUM_5)
#define GPIO_OUTPUT_LED     (GPIO_NUM_16)
#define	GPIO_INPUT_SENSOR	(GPIO_NUM_4)
#else
#define	GPIO_INPUT_BUTTON	(GPIO_NUM_13)
//#define	GPIO_INPUT_BUTTON	(GPIO_NUM_0)
#define GPIO_OUTPUT_RELE    (GPIO_NUM_5)
#define GPIO_OUTPUT_LED     (GPIO_NUM_15)
#define	GPIO_INPUT_SENSOR	(GPIO_NUM_4)
#endif /* USING_RFID */

#ifdef USING_DB9
#define GPIO_OUTPUT_DB9_0		(GPIO_NUM_18)
#define GPIO_OUTPUT_DB9_1		(GPIO_NUM_19)
#define GPIO_OUTPUT_DB9_2		(GPIO_NUM_21)
#define GPIO_OUTPUT_DB9_3		(GPIO_NUM_22)
#define GPIO_OUTPUT_DB9_4		(GPIO_NUM_23)
#define GPIO_OUTPUT_DB9_5		(GPIO_NUM_25)
#define GPIO_OUTPUT_DB9_6		(GPIO_NUM_26)
#define GPIO_OUTPUT_DB9_7		(GPIO_NUM_27)
#endif  /* USING_DB9 */

typedef enum
{
	GPIO_CONFIG_ID_RELE = 0,
	GPIO_CONFIG_ID_LED ,
	GPIO_CONFIG_ID_BUTTON,

#ifdef USING_DB9
	GPIO_CONFIG_DB9_0,
	GPIO_CONFIG_DB9_1,
	GPIO_CONFIG_DB9_2,
	GPIO_CONFIG_DB9_3,
	GPIO_CONFIG_DB9_4,
	GPIO_CONFIG_DB9_5,
	GPIO_CONFIG_DB9_6,
	GPIO_CONFIG_DB9_7,
#endif /* USING_DB9 */

	GPIO_CONFIG_ID_MAX
} GPIO_Config_Id_t;

typedef struct
{
	bool initialized;
	gpio_config_t configTable[GPIO_CONFIG_ID_MAX];
} GPIO_Cfg_t;

GPIO_Cfg_t * GPIO_Cfg_getInstance();
CubeeErrorCode_t GPIO_Cfg_init(GPIO_Cfg_t* this);

#endif /* COMPONENTS_DRV_DIO_DIO_CFG_H_ */
