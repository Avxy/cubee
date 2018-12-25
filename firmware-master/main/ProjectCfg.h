/*
 * ProjectCfg.h
 *
 * Pinout for different kinds of CUBEEs:
 * ---------------------- ENERGY ---------------------------------------
 * BUTTON - (GPIO_NUM_13)
 * RELE - (GPIO_NUM_5)
 * LED - (GPIO_NUM_15)
 * SENSOR    (GPIO_NUM_4)
 * ---------------------- RFID ---------------------------------------
 * BUTTON - (GPIO_NUM_0)
 * RELE - (GPIO_NUM_5)
 * LED - (GPIO_NUM_16)
 * SPI MOSI - GPIO_NUM_13
 * SPI MISO - GPIO_NUM_12
 * SPI CLK - GPIO_NUM_14
 * SPI RFID CS - GPIO_NUM_15
 * ---------------------- DB9---------------------------------------
 * BUTTON - (GPIO_NUM_13)
 * RELE - (GPIO_NUM_5)
 * LED - (GPIO_NUM_15)
 * OUTPUT_DB9_0 - (GPIO_NUM_18)
 * OUTPUT_DB9_1 - (GPIO_NUM_19)
 * OUTPUT_DB9_2 - GPIO_NUM_21)
 * OUTPUT_DB9_3 - (GPIO_NUM_22)
 * OUTPUT_DB9_4 - (GPIO_NUM_23)
 * OUTPUT_DB9_5 - (GPIO_NUM_25)
 * OUTPUT_DB9_6 - (GPIO_NUM_26)
 * OUTPUT_DB9_7 - (GPIO_NUM_27)
 * --------------------- SENSOR ---------------------------------------
 * BUTTON - (GPIO_NUM_13)
 * RELE - (GPIO_NUM_5)
 * LED - (GPIO_NUM_15)
 * SENSOR  - (GPIO_NUM_4)
 * I2C_SDA - (21)
 * I2C_SCL - (22)
 * -------------------------------------------------------------------------
 *
 *  Created on: Jun 27, 2017
 *      Author: Alex Fernandes
 */

#ifndef MAIN_PROJECTCFG_H_
#define MAIN_PROJECTCFG_H_

/* TEST FLAGS */
#define xTEST_SENSOR_MGR
#define xTEST_STORAGE_MGR
#define TEST_MEMORY_MGR
#define xTEST_I2C
#define xTEST_WAN_CONTROLLER
#define xTEST_BLUETOOTH
#define xTEST_SPI_MASTER
#define xTEST_SPI_SLAVE
#define xTEST_RFID
#define xTEST_DB9
#define xTEST_MQTT

/* CUBEE general settings  */
#define CUBEE_CONFIG_SERVICE_VALUE_SIZE	(512)
#define ID_CUBEE_MAX_SIZE				(25)
#define CUBEE_NAME_MAX_SIZE				(24)
#define SSID_MAX_SIZE					(32)
#define PASSWORD_MAX_SIZE				(64)
#define CUBEE_DEFAULT_ID				"0"
#define DB9_RULE_STATE_MAX				(8)		/*!< Defines the maximum number of states allowed for a DB9 rule */

/* CUBEE specific modules activation */
#define XUSING_RFID
#define xUSING_SENSOR_MGR
#define USING_DB9

/* CUBEE sensors settings */
#ifdef USING_SENSOR_MGR
#define xUSING_ACCELEROMETER
#define USING_CURRENT_SENSOR
#endif /* USING_SENSOR_MGR */

/* CUBEE Real-Time settings */

#define CUBEE_APP_TASK_CORE				(1)
#define CUBEE_APP_TASK_PRIORITY			(5)
#define CUBEE_APP_TASK_DELAY			(1000 / portTICK_PERIOD_MS)
#define CUBEE_APP_TASK_INFO_TIMEOUT		(60000 / portTICK_PERIOD_MS)

#define SENSOR_MGR_TASK_CORE			(1)
#define SENSOR_MGR_TASK_PRIORITY		(5)
#define SENSOR_MGR_TASK_DELAY			(1000 / portTICK_PERIOD_MS)
#define SENSOR_MGR_TASK_INFO_TIMEOUT	(60000 / portTICK_PERIOD_MS)

#define MQTT_TASK_CORE					(1)
#define MQTT_TASK_PRIORITY				(4)
#define MQTT_TASK_DELAY					(1000 / portTICK_PERIOD_MS)
#define MQTT_TASK_INFO_TIMEOUT			(60000 / portTICK_PERIOD_MS)

#define BLUETOOTH_TASK_CORE				(1)
#define BLUETOOTH_TASK_PRIORITY			(3)
#define BLUETOOTH_TASK_DELAY			(500 / portTICK_PERIOD_MS)
#define BLUETOOTH_TASK_INFO_TIMEOUT		(60000 / portTICK_PERIOD_MS)

#define WIFI_CTRL_TASK_CORE				(1)
#define WIFI_CTRL_TASK_PRIORITY			(3)
#define WIFI_CTRL_TASK_DELAY			(500 / portTICK_PERIOD_MS)
#define WIFI_CTRL_TASK_INFO_TIMEOUT		(60000 / portTICK_PERIOD_MS)

#define RFID_APP_TASK_CORE				(1)
#define RFID_APP_TASK_PRIORITY			(2)
#define RFID_APP_TASK_DELAY				(50 / portTICK_PERIOD_MS)
#define RFID_APP_TASK_INFO_TIMEOUT		(60000 / portTICK_PERIOD_MS)

#define DB9_APP_TASK_CORE				(1)
#define DB9_APP_TASK_PRIORITY			(2)
#define DB9_APP_TASK_DELAY				(50 / portTICK_PERIOD_MS)
#define DB9_APP_TASK_INFO_TIMEOUT		(60000 / portTICK_PERIOD_MS)

#endif /* MAIN_PROJECTCFG_H_ */
