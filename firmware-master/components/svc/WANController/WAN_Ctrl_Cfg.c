
/**
 * @file WAN_Ctrl_Cfg.h
 * @version 2.0
 * @author Alex Fernandes
 * @date October 16, 2017
 **************************************************************************
 *
 * @brief  WAN Controller Configuration
 * This module has configurations used by the WAN Controller.
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   07-Jun-2017    Alex Fernandes
 * * Original version
 *
 *  Revision: 2.0   16-Oct-2017    Alex Fernandes
 * * Configuration refactoring for integration with MQTT service
 ******************************************************************/

/* Modules Dependencies */
#include "WAN_Ctrl_Cfg.h"


static WAN_Ctrl_Cfg_t wanCtrlCfgInstance =
{
		.initialized = false,
};


CubeeErrorCode_t WAN_Ctrl_Cfg_init(WAN_Ctrl_Cfg_t * this)
{
	/* Verify if the WAN controller is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	this->initialized = true;
	return CUBEE_ERROR_OK;
}

WAN_Ctrl_Cfg_t* WAN_Ctrl_Cfg_getInstance()
{
	return &wanCtrlCfgInstance;
}

bool WAN_Ctrl_Cfg_isInitialized(WAN_Ctrl_Cfg_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

