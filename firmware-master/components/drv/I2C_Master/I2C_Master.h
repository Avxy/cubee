/*
 * I2C_Master.h
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#ifndef I2C_Master_H_
#define I2C_Master_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <string.h>

#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "I2C_Master_Cfg.h"

/*! Enum to index the I2C_Master channels */
typedef enum
{
	I2C_ID_ACCELEROMETER = 0,

	I2C_SLAVE_ID_MAX,

} I2C_Slave_Id_t;


/*! Struct to instantiate the I2C_Master component */
typedef struct I2C_Master
{
	bool initialized;								/*!< Indicates if the I2C_Master component was initialized */

	I2C_Slave_Cfg_t slaveCfg[I2C_SLAVE_ID_MAX]; 	/*!< Table with I2C slaves to be connected at the master bus */
	I2C_Master_Cfg_t *I2C_MasterCfg;				/*!< Pointer to the configurations used by the I2C_Master instance */
} I2C_Master_t;

/**
 * @brief This function initializes the I2C_Master instance and all its dependencies.
 *
 * @param this - pointer to I2C_Master component instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the component I2C_Master initialization was successful.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t I2C_Master_init(I2C_Master_t* this);

/**
* @brief This function gets the I2C_Master configuration instance.
*
* @return pointer to the I2C_Master configuration instance.
*/
I2C_Master_Cfg_t * I2C_Master_getConfig(I2C_Master_t* this);

/**
* @brief This function gets the I2C_Master instance.
*
* @return pointer to I2C_Master component instance.
*/
I2C_Master_t * I2C_Master_getInstance();

/**
 * @brief This function checks if the I2C_Master instance is already initialized.
 *
 * @return
 * @arg true, if the I2C_Master instance is initialized.
 * @arg false, otherwise.
*/
bool I2C_Master_isInitialized(I2C_Master_t* this);

CubeeErrorCode_t I2C_Master_write(I2C_Master_t* this, I2C_Slave_Id_t slaveId, uint8_t regAddress, uint8_t* dataWrite, uint16_t size);

CubeeErrorCode_t I2C_Master_read(I2C_Master_t* this, I2C_Slave_Id_t slaveId, uint8_t regAddress, uint8_t* dataRead, uint16_t size);

CubeeErrorCode_t I2C_Master_writeReg(I2C_Master_t* this, I2C_Slave_Id_t slaveId, uint8_t regAddress, uint8_t dataWrite);

CubeeErrorCode_t I2C_Master_testCommunication(I2C_Master_t* this, I2C_Slave_Id_t slaveId);


#ifdef __cplusplus
}
#endif

#endif /* I2C_Master_H_ */
