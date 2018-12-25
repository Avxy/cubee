
/* Modules Dependencies */
#include "Wifi_Ctrl_Cfg.h"


static Wifi_Ctrl_Cfg_t WifiCtrlCfgInstance =
{
		.wifiSSID = WIFI_CTRL_DEFAULT_WIFI_SSID,
		.wifiPassword = WIFI_CTRL_DEFAULT_WIFI_PSWD,
		.initialized = false,
};


CubeeErrorCode_t Wifi_Ctrl_Cfg_init(Wifi_Ctrl_Cfg_t * this)
{
	/* Verify if the Wifi controller is already initialized*/
	if(this->initialized != false)
	{
		return CUBEE_ERROR_OK;
	}

	this->initialized = true;
	return CUBEE_ERROR_OK;
}

Wifi_Ctrl_Cfg_t* Wifi_Ctrl_Cfg_getInstance()
{
	return &WifiCtrlCfgInstance;
}

bool Wifi_Ctrl_Cfg_isInitialized(Wifi_Ctrl_Cfg_t * this)
{
	if(this == NULL)
	{
		return false;
	}

	return this->initialized;
}

