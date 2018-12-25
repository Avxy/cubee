/*
 * RFID_Reader_Cfg.c
 *
 *  Created on: 28 de jul de 2017
 *      Author: Samsung
 */

#include "RFID_Reader_Cfg.h"


static RFID_Reader_Cfg_t RFID_ReaderCfgInstance =
{
		.initialized = false,
};

CubeeErrorCode_t RFID_Reader_Cfg_init(RFID_Reader_Cfg_t* this)
{
	this->initialized = true;

	return  CUBEE_ERROR_OK;
}

RFID_Reader_Cfg_t * RFID_Reader_Cfg_getInstance()
{
	return  &RFID_ReaderCfgInstance;
}


