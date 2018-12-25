/**
 * @file RfidApp_Cfg.h
 * @version 1.0
 * @author Alex Fernandes
 * @date June 06, 2017
 **************************************************************************
 *
 * @brief  Wifi Controller Configuration
 * This module has configurations used by the Wifi Controller.
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   06-Jun-2017    Alex Fernandes
 * * Original version
 ******************************************************************/


/* Modules Dependencies */
#include "RfidApp_Cfg.h"


static RfidApp_Cfg_t rfidAppCfgInstance =
{
		.initialized = false,
};

CubeeErrorCode_t RfidApp_Cfg_init(RfidApp_Cfg_t * this)
{
	/* Verify if the Wifi controller is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	this->initialized = true;
	return CUBEE_ERROR_OK;
}

RfidApp_Cfg_t* RfidApp_Cfg_getInstance(void)
{
	return &rfidAppCfgInstance;
}

bool RfidApp_Cfg_isInitialized(RfidApp_Cfg_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

