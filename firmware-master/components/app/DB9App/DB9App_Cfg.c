/**
 * @file DB9App_Cfg.c
 * @version 1.0
 * @author Alex Fernandes
 * @date September 29, 2017
 **************************************************************************
 *
 * @brief  DB9 App configuration
 * This module has configurations used by the DB9 application.
 *
 * @section References
 * @ref 1.
 *
 ***************************************************************************
 * @section Revisions:
 *
 * Revision: 1.0   29-Sep-2017    Alex Fernandes
 * * Original version
 ******************************************************************/


/* Modules Dependencies */
#include "DB9App_Cfg.h"


static DB9App_Cfg_t DB9AppCfgInstance =
{
		.initialized = false,
};

CubeeErrorCode_t DB9App_Cfg_init(DB9App_Cfg_t * this)
{
	/* Verify if the module configuration is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	this->initialized = true;
	return CUBEE_ERROR_OK;
}

DB9App_Cfg_t* DB9App_Cfg_getInstance(void)
{
	return &DB9AppCfgInstance;
}

bool DB9App_Cfg_isInitialized(DB9App_Cfg_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

