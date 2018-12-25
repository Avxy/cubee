/*
 * PANController_Cfg.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "PANController_Cfg.h"


static PANController_Cfg panControllerCfg =
{
		.initialized = false,
};

CubeeErrorCode_t PANController_Cfg_init(PANController_Cfg * this)
{
	/* Verify if the Notifier is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	this->initialized = true;

	return CUBEE_ERROR_OK;
}

PANController_Cfg * PANController_Cfg_getInstance(void)
{
	return &panControllerCfg;
}


