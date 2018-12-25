/*
 * DIO.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "I2C_Master.h"
#include "I2C_Master_Cfg.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"

#define LOG_HEADER 								"[I2C_Master DRV] "

#define 										xDEBUG_I2C_Master

#define ACK_VAL    0x0         /*!< I2C ack value */
#define NACK_VAL   0x1         /*!< I2C nack value */

/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/
I2C_Master_t i2cMasterInstance =
{
	.initialized = false,
	.slaveCfg =
	{
			[I2C_ID_ACCELEROMETER] =
				{
					.port = I2C_MASTER_PORT,
					.slaveAdress = I2C_ACELEROMETER_ADDRESS,
				},
	},
};


/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/

/**
 * @brief test function to show buffer
 */

#ifdef DEBUG_I2C_Master
static void __I2C_Master_printBuffer(uint8_t* buf, uint16_t len);
#endif


/************************************************************************************************/
/* Public API  Implementation */
/************************************************************************************************/

I2C_Master_Cfg_t * I2C_Master_getConfig(I2C_Master_t* this)
{
	return  this->I2C_MasterCfg;
}

I2C_Master_t * I2C_Master_getInstance()
{
	return  &i2cMasterInstance;
}

CubeeErrorCode_t I2C_Master_init(I2C_Master_t* this)
{

	if(I2C_Master_isInitialized(this))
	{
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing I2C Master");

	this->I2C_MasterCfg = I2C_Master_Cfg_getInstance();
	if(I2C_Master_Cfg_init(this->I2C_MasterCfg) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "I2C Master configuration initialization failed\n");
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER,"I2C Configuration initialized");

	if(i2c_param_config(I2C_MASTER_PORT, &(this->I2C_MasterCfg->masterCfg)) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"I2C driver initialization failed, error at the master port configuration ");
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER,"I2C Config initialized");

	if (i2c_driver_install(I2C_MASTER_PORT, this->I2C_MasterCfg->masterCfg.mode, 0, 0, 0) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"I2C driver initialization failed, error installing I2C driver ");
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER,"I2C driver installed");

	/* Test communication with all slaves defined*/
	for(uint16_t slaveId = 0; slaveId < I2C_SLAVE_ID_MAX; slaveId++)
	{
		if(I2C_Master_testCommunication(this,slaveId) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER, "Failure communicating with I2C slave %d", slaveId);
		}
	}
	this->initialized = true;
	ESP_LOGI(LOG_HEADER,"I2C_Master Initialized\n");
	vTaskDelay(5000 / portTICK_PERIOD_MS);

	return CUBEE_ERROR_OK;
}

bool I2C_Master_isInitialized(I2C_Master_t* this)
{
	return  this->initialized;
}

CubeeErrorCode_t I2C_Master_write(I2C_Master_t* this, I2C_Slave_Id_t slaveId, uint8_t regAddress, uint8_t* dataWrite, uint16_t size)
{
	i2c_cmd_handle_t cmd;
	esp_err_t espError;

	if((this == NULL) || (slaveId >= I2C_SLAVE_ID_MAX) || (dataWrite == NULL) || (size == 0))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER writing I2C slave");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (this->slaveCfg[slaveId].slaveAdress << 1) | I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, regAddress, true);
	i2c_master_write(cmd, dataWrite, size, true);
	i2c_master_stop(cmd);
	espError = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	if(espError != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error writing register %02x from  slave address %02x, Error Code: %02X", regAddress,  this->slaveCfg[slaveId].slaveAdress, espError);
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef DEBUG_I2C_Master
	ESP_LOGI(LOG_HEADER,"Register %02x successfully wrote to  slave address: %02x.", regAddress,this->slaveCfg[slaveId].slaveAdress);
#endif


	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t I2C_Master_read(I2C_Master_t* this, I2C_Slave_Id_t slaveId, uint8_t regAddress, uint8_t* dataRead, uint16_t size)
{
	 i2c_cmd_handle_t cmd;
	 esp_err_t espError;

	if((this == NULL) || (slaveId >= I2C_SLAVE_ID_MAX) || (dataRead == NULL) || (size == 0))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading I2C slave register");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	 /* Clean buffer */
	 memset(dataRead,0,size);

	if(regAddress > 0)
	{
		/* Informs the slave and register address to be read */
		cmd = i2c_cmd_link_create();
	    i2c_master_start(cmd);
	    i2c_master_write_byte(cmd, ( this->slaveCfg[slaveId].slaveAdress << 1 ) |  I2C_MASTER_WRITE, true);
	    i2c_master_write_byte(cmd, regAddress, true);
	    i2c_master_stop(cmd);
	    /* Send commands queued */
	    espError = i2c_master_cmd_begin(this->slaveCfg[slaveId].port, cmd, 1000 / portTICK_RATE_MS);
	    /* End I2C connection */
	    i2c_cmd_link_delete(cmd);

		if(espError != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error reading data from slave address: %02x, beginning at register %02x , size: %02x.", this->slaveCfg[slaveId].slaveAdress, regAddress, size);
			return CUBEE_ERROR_UNDEFINED;
		}
	}

    /* Read data begins at the specified register */
	cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( this->slaveCfg[slaveId].slaveAdress << 1 ) |  I2C_MASTER_READ, true);
    if(size > 1)
    {
    	i2c_master_read(cmd, dataRead, size - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, dataRead + (size - 1), NACK_VAL);
    i2c_master_stop(cmd);
    /* Send commands queued */
    espError = i2c_master_cmd_begin(this->slaveCfg[slaveId].port, cmd, 1000 / portTICK_RATE_MS);
    /* End I2C connection */
    i2c_cmd_link_delete(cmd);

	if(espError != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error reading data from slave address: %02x, beginning at register %02x , size: %02x.", this->slaveCfg[slaveId].slaveAdress, regAddress, size);
		return CUBEE_ERROR_UNDEFINED;
	}


#ifdef DEBUG_I2C_Master
	ESP_LOGI(LOG_HEADER,"Data successfully read from slave address: %02x, beginning at register %02x , size: %02x.", this->slaveCfg[slaveId].slaveAdress, regAddress, size);
	__I2C_Master_printBuffer(dataRead,size);
#endif

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t I2C_Master_writeReg(I2C_Master_t* this, I2C_Slave_Id_t slaveId, uint8_t regAddress, uint8_t dataWrite)
{
	i2c_cmd_handle_t cmd;
	esp_err_t espError;

	if((this == NULL) || (slaveId >= I2C_SLAVE_ID_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER writing I2C slave register");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (this->slaveCfg[slaveId].slaveAdress << 1) | I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, regAddress, true);
	i2c_master_write_byte(cmd, dataWrite, true);
	i2c_master_stop(cmd);
	espError = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 1000/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	if(espError != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error writing register %02x from  slave address %02x, Error Code: %02X", regAddress,  this->slaveCfg[slaveId].slaveAdress, espError);
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef DEBUG_I2C_Master
	ESP_LOGI(LOG_HEADER,"Register %02x successfully wrote to  slave address: %02x.", regAddress,this->slaveCfg[slaveId].slaveAdress);
#endif


	return CUBEE_ERROR_OK;
}


CubeeErrorCode_t I2C_Master_testCommunication(I2C_Master_t* this, I2C_Slave_Id_t slaveId)
{
	i2c_cmd_handle_t cmd;
	esp_err_t espError;

	if((this == NULL) || (slaveId >= I2C_SLAVE_ID_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER verifying communication with I2C slave");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Build test command */
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (this->slaveCfg[slaveId].slaveAdress << 1) | I2C_MASTER_WRITE, true);
	i2c_master_stop(cmd);

	/* Send test command to slave */
	espError = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	if(espError == ESP_OK)
	{
#ifdef DEBUG_I2C_Master
		ESP_LOGI(LOG_HEADER,"Communication with I2C slave successfully verified, slave address: %02x", this->slaveCfg[slaveId].slaveAdress);
#endif
		return CUBEE_ERROR_OK;
	}

	/* Communication failed */
#ifdef DEBUG_I2C_Master
	if(espError == ESP_ERR_TIMEOUT)
	{
		ESP_LOGE(LOG_HEADER,"Error testing slave communication, I2C bus is busy, slave address: %02x", this->slaveCfg[slaveId].slaveAdress);
	}
	else if(espError == ESP_ERR_INVALID_STATE)
	{
		ESP_LOGE(LOG_HEADER,"Error testing slave communication, I2C driver not installed or not in master mode, slave address: %02x", this->slaveCfg[slaveId].slaveAdress);
	}
	else if(espError == ESP_FAIL)
	{
		ESP_LOGE(LOG_HEADER,"Error testing slave communication, slave doesn't ACK the transfer, slave address: %02x", this->slaveCfg[slaveId].slaveAdress);
	}
#endif

	return CUBEE_ERROR_UNDEFINED;
}

/************************************************************************************************/
/* Internal Functions Implementation  */
/************************************************************************************************/

#ifdef DEBUG_I2C_Master
static void __I2C_Master_printBuffer(uint8_t* buf, uint16_t len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if (( i + 1 ) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
#endif


