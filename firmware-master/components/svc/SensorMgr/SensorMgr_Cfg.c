/*
 * PowerMgr_Cfg.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "SensorMgr_Cfg.h"

static SensorMgr_Cfg_t sensorMgrCfg =
{
		.initialized = false,
};

SensorMgr_Cfg_t * SensorMgr_Cfg_getInstance()
{
	return  &sensorMgrCfg;
}

CubeeErrorCode_t SensorMgr_Cfg_init(SensorMgr_Cfg_t* this)
{
	/* Verify if the PowerMgr is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	this->initialized = true;

	return CUBEE_ERROR_OK;
}
