/*
 * IOMgr_Cfg.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "IOMgr_Cfg.h"


static IOMgr_Cfg_t ioMgrCfgInstance =
{
		.initialized = false,
};

CubeeErrorCode_t IOMgr_Cfg_init(IOMgr_Cfg_t* this)
{
	this->initialized = true;

	return  CUBEE_ERROR_OK;
}

IOMgr_Cfg_t * IOMgr_Cfg_getInstance()
{
	return  &ioMgrCfgInstance;
}


