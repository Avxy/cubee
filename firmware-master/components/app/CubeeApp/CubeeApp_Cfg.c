/* Modules Dependencies */
#include "CubeeApp_Cfg.h"

static CubeeApp_Cfg_t notifierCfg =
{
		.initialized = false,
};

CubeeApp_Cfg_t * CubeeApp_Cfg_getInstance(void)
{
	return &notifierCfg;
}

CubeeErrorCode_t CubeeApp_Cfg_init(CubeeApp_Cfg_t * this)
{
	/* Verify if the CubeeApp is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	this->initialized = true;

	return CUBEE_ERROR_OK;
}

bool CubeeApp_Cfg_isInitialized(CubeeApp_Cfg_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}
