/*
 * IOMgr.c
 *
 *  Created on: 23 de mai de 2017
 *      Author: Samsung
 */

#include "IOMgr.h"
#include "IOMgr_Cfg.h"
#include "../../drv/GPIO/GPIO.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"
#include "esp_log.h"

#define LOG_HEADER		"[IO_MGR SVC] "

/************************************************************************************************/
/* Local Variables */
/************************************************************************************************/
static uint8_t ledStatus;

IOMgr_t ioMgrInstance =
{
	.initialized = false,
};

#ifdef USING_DB9
static GPIO_ID_t db9Output[8] = {GPIO_DB9_0, GPIO_DB9_1, GPIO_DB9_2, GPIO_DB9_3, GPIO_DB9_4, GPIO_DB9_5, GPIO_DB9_6, GPIO_DB9_7};
#endif /* USING_DB9 */

/************************************************************************************************/
/* Internal Functions Definition */
/************************************************************************************************/


/************************************************************************************************/
/* Public API  Implementation */

CubeeErrorCode_t IOMgr_init(IOMgr_t* this)
{
#ifdef USING_ACCELEROMETER
	uint8_t accelerometerActvationMax = 5;
	uint8_t accelerometerActivationCount = 0;
#endif /* USING_ACCELEROMETER */

	if (IOMgr_isInitialized(this)) {
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER, "Initializing IOMgr");

	this->ioMgrCfg = IOMgr_Cfg_getInstance();
	if(IOMgr_Cfg_init(this->ioMgrCfg) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "IOMgr_Cfg initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	this->gpio = GPIO_getInstance();
	if(GPIO_init(this->gpio) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "IOMgr initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

	this->adc = ADC_getInstance();
	if(ADC_init(this->adc) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "IOMgr initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef USING_ACCELEROMETER
	this->i2c = I2C_Master_getInstance();
	if(I2C_Master_init(this->i2c) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "IOMgr initialization failed");
		return CUBEE_ERROR_UNDEFINED;
	}
	while((I2C_Master_writeReg(this->i2c, I2C_ID_ACCELEROMETER, PWR_MGMT_1, 0x00) != CUBEE_ERROR_OK) && (accelerometerActivationCount <= accelerometerActvationMax))
	{
		ESP_LOGE(LOG_HEADER, "IOMgr initialization failed, error activating accelerometer");
		accelerometerActivationCount++;
	}
	if(accelerometerActivationCount > accelerometerActvationMax)
	{
		/* Wasn't possible activate the accelerometer */
		return CUBEE_ERROR_UNDEFINED;
	}
#endif /* USING_ACCELEROMETER */

	/* Power on LED */
	ledStatus = PIN_OFF;
	IOMgr_toogleLED(this);

	this->initialized = true;

	ESP_LOGI(LOG_HEADER, "IOMgr Initialized");

	return CUBEE_ERROR_OK;
}

IOMgr_Cfg_t * IOMgr_getConfig(IOMgr_t* this)
{
	return  this->ioMgrCfg;
}

IOMgr_t * IOMgr_getInstance()
{
	return  &ioMgrInstance;
}

CubeeErrorCode_t IOMgr_activate(IOMgr_t* this)
{
	CubeeErrorCode_t error = CUBEE_ERROR_OK;

	error = GPIO_write(this->gpio,GPIO_RELE,PIN_ON);
	if(error != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Activation failed");
	}

	return  error;
}

CubeeErrorCode_t IOMgr_deactivate(IOMgr_t* this)
{
	CubeeErrorCode_t error = CUBEE_ERROR_OK;

	error = GPIO_write(this->gpio,GPIO_RELE ,PIN_OFF);
	if(error != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Deactivation failed");
	}

	return  error;
}

bool IOMgr_isActivated(IOMgr_t* this)
{
	if(GPIO_read(this->gpio,GPIO_RELE) == (uint32_t) PIN_ON)
	{
		return true;
	}

	return false;
}

bool IOMgr_isInitialized(IOMgr_t* this)
{
	return  this->initialized;
}

bool IOMgr_hasButtonCmd(IOMgr_t* this)
{
	uint8_t buttonCmd = 0x00;

	GPIO_readButton(this->gpio,&buttonCmd);


	if(buttonCmd == 0x01)
	{
		ESP_LOGI(LOG_HEADER,"Button command detected");
		return true;
	}

	return false;
}

CubeeErrorCode_t  IOMgr_toogleLED( IOMgr_t* this)
{
	CubeeErrorCode_t error = CUBEE_ERROR_OK;

	if(ledStatus == (uint32_t) PIN_ON)
	{
		error = GPIO_write(this->gpio,GPIO_LED, PIN_OFF);
		if(error == CUBEE_ERROR_OK)
		{
			ledStatus = PIN_OFF;
		}

		return error;
	}

	error = GPIO_write(this->gpio,GPIO_LED, PIN_ON);
	if(error == CUBEE_ERROR_OK)
	{
		ledStatus = PIN_ON;
	}

	return error;
}

#ifdef USING_CURRENT_SENSOR
CubeeErrorCode_t  IOMgr_readCurrent( IOMgr_t* this, int16_t* outputCurrent)
{
	(*outputCurrent) = ADC_read(this->adc,ADC_CURRENT_SENSOR_ID);
	if((*outputCurrent) < 0)
	{
		ESP_LOGE(LOG_HEADER, "Error reading current sensor");
		return CUBEE_ERROR_UNDEFINED;
	}

	return CUBEE_ERROR_OK;
}
#endif /* USING_CURRENT_SENSOR */


#ifdef USING_ACCELEROMETER
CubeeErrorCode_t  IOMgr_readAccelerometer( IOMgr_t* this, int16_t* outputZ)
{
	uint8_t axisBuffer[ACCEL_AXIS_DATA_SIZE];

	if(I2C_Master_testCommunication(this->i2c, I2C_ID_ACCELEROMETER) != CUBEE_ERROR_OK)
	{
		return CUBEE_ERROR_UNDEFINED;
	}

	if(I2C_Master_read(this->i2c, I2C_ID_ACCELEROMETER, ACCEL_ZOUT, axisBuffer, ACCEL_AXIS_DATA_SIZE) != CUBEE_ERROR_OK)
	{
		return CUBEE_ERROR_UNDEFINED;
	}

	(*outputZ) = ((int16_t)((axisBuffer[0] << 8) | axisBuffer[1]));

	return CUBEE_ERROR_OK;
}
#endif /* USING_ACCELEROMETER */

#ifdef USING_DB9
CubeeErrorCode_t  IOMgr_setDB9Output( IOMgr_t* this, uint8_t value)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER setting DB9 output");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	return GPIO_writeByte(this->gpio, db9Output, value, 8);
}
#endif /* USING_DB9 */

