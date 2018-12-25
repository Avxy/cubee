/*
 * I2C_Master_Cfg.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "I2C_Master_Cfg.h"

#define	I2C_MASTER_SDA_IO			(21)
#define I2C_MASTER_FREQ_HZ			(1000000)
#define I2C_MASTER_SCL_IO			(22)

static I2C_Master_Cfg_t I2C_MasterCfgInstance =
{
		.initialized = false,
		.masterCfg =
				{
					    .mode = I2C_MODE_MASTER,
					    .sda_io_num = I2C_MASTER_SDA_IO,
					    .sda_pullup_en = GPIO_PULLUP_ENABLE,
					    .scl_io_num = I2C_MASTER_SCL_IO,
					    .scl_pullup_en = GPIO_PULLUP_ENABLE,
					    .master.clk_speed = I2C_MASTER_FREQ_HZ,

				},
};


I2C_Master_Cfg_t * I2C_Master_Cfg_getInstance()
{
	return  &I2C_MasterCfgInstance;
}

CubeeErrorCode_t I2C_Master_Cfg_init(I2C_Master_Cfg_t* this)
{
	this->initialized = true;

	return  CUBEE_ERROR_OK;
}
