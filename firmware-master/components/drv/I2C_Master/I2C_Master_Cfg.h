/*
 * I2C_Master_Cfg.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef I2C_Master_CFG_H_
#define I2C_Master_CFG_H_

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <string.h>

#include "driver/i2c.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define I2C_ACELEROMETER_ADDRESS	(0x68)
#define I2C_MASTER_PORT				(I2C_NUM_1)

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	i2c_port_t port;
	uint8_t slaveAdress;
}I2C_Slave_Cfg_t;

typedef struct
{
	bool initialized;
	i2c_config_t masterCfg;
} I2C_Master_Cfg_t;

I2C_Master_Cfg_t* this, * I2C_Master_Cfg_getInstance();
CubeeErrorCode_t I2C_Master_Cfg_init(I2C_Master_Cfg_t* this);




#ifdef __cplusplus
}
#endif

#endif /* I2C_Master_CFG_H_ */
