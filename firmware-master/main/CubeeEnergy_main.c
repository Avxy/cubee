/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "../components/app/CubeeApp/CubeeApp.h"
#include "../components/util/ErrorCode/CUBEE_ErrorCode.h"

#define LOG_HEADER 	"[MAIN_TASK] "

static TaskHandle_t * mainTask;


#ifdef TEST_SENSOR_MGR
#include "../components/svc/SensorMgr/SensorMgr.h"
#endif /* TEST_SENSOR_MGR */

#ifdef TEST_I2C
#include "../components/drv/I2C_Master/I2C_Master.h"
static uint8_t acelerometerRegistersBuffer[14];
#endif /* TEST_I2C */

#ifdef TEST_STORAGE_MGR
#include "../components/svc/StorageMgr/StorageMgr.h"
#define SAMPLE_NUMBER  		10
#define ALARM_NUMBER 	 	10
static StorageMgr_SampleUnion_t sampleBufferWrite[SAMPLE_NUMBER];
static StorageMgr_SampleUnion_t sampleBufferRead[SAMPLE_NUMBER];
static StorageMgr_AlarmUnion_t alarmBufferWrite[ALARM_NUMBER];
static StorageMgr_AlarmUnion_t alarmBufferread[ALARM_NUMBER];
static CubeeConfig_t cubeeCfg = {0};
static char cubeeName[CUBEE_NAME_MAX_SIZE] = "CUBEE-FW";
static char cubeeId[ID_CUBEE_MAX_SIZE] = "597608455f446b4fc937d318";
static char password[PASSWORD_MAX_SIZE] = "Lacina_21011901";
static char ssid[SSID_MAX_SIZE] = "Lacina";
#endif  /* TEST_STORAGE_MGR */

#ifdef TEST_MEMORY_MGR
#include "../components/drv/MemoryMgr/MemoryMgr.h"
#define SEND_BUFFER_SIZE  	600
#define RCV_BUFFER_SIZE  	MEMORY_MGR_DATA_BUFFER_MAX_SIZE
static uint8_t sendBuffer[SEND_BUFFER_SIZE];
static uint8_t rcvBuffer[RCV_BUFFER_SIZE];
#endif  /* TEST_STORAGE_MGR */

#ifdef TEST_WAN_CONTROLLER
#include "../components/svc/WANController/WAN_Ctrl.h"
#include "../components/svc/StorageMgr/StorageMgr.h"
WAN_Ctrl_Data_Packet_t wanDataPacket;
#endif /* TEST_WAN_CONTROLLER */

#ifdef TEST_BLUETOOTH
#include "../components/drv/Bluetooth/Bluetooth.h"
#include "../components/svc/StorageMgr/StorageMgr.h"

#endif /* TEST_BLUETOOTH */

#ifdef TEST_SPI_MASTER
#include "../components/drv/SPI/SPI_Master.h"
#define SPI_MASTER_BUFFER_SIZE	(258)
uint8_t sendbuf[SPI_MASTER_BUFFER_SIZE];
uint8_t recvbuf[SPI_MASTER_BUFFER_SIZE];
#define GPIO_HANDSHAKE 2
static xQueueHandle rdySem;
#endif /* TEST_SPI_MASTER */

#ifdef TEST_SPI_SLAVE
#define GPIO_HANDSHAKE 2
#include "driver/spi_slave.h"
#include "../components/drv/SPI/SPI_Slave.h"
#define SPI_MASTER_BUFFER_SIZE	(128)
uint8_t sendbuf[SPI_MASTER_BUFFER_SIZE];
uint8_t recvbuf[SPI_MASTER_BUFFER_SIZE];

#endif  /* TEST_SPI_SLAVE */

#ifdef TEST_RFID
#include "../components/svc/RFID_Reader/RFID_Reader.h"
#include "../components/app/RfidApp/RfidApp.h"
#endif

#ifdef TEST_DB9
#include "../components/app/DB9App/DB9App.h"
#include "../components/svc/IOMgr/IOMgr.h"
#include "../components/svc/StorageMgr/StorageMgr.h"
static DB9App_rule_t db9Rule;
#endif

#ifdef TEST_MQTT
#include "../components/svc/MQTT/MQTT_Network.h"
#include "../components/drv/Wifi_Ctrl/Wifi_Ctrl.h"
#endif

#ifdef TEST_BLUETOOTH
void __Bluetooth_test_task()
{

	Bluetooth_t * bluetooth = Bluetooth_getInstance();
	Bluetooth_init(bluetooth);


	/* updates Cubee id used to WAN requests */
	StorageMgr_Init();
	Bluetooth_setName(bluetooth, StorageMgr_getCurrentCubeeCfg()->cubeeName);

	for (;;)
	{
		ESP_LOGI(LOG_HEADER,"Running Bluetooth Test Application...\n");

		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
#endif /* TEST_BLUETOOTH */

#ifdef TEST_WAN_CONTROLLER
void __WAN_Ctrl_test_task()
{
	WAN_Ctrl_t * wanCtrl;
	char * wifiPassword =	"Lacina_21011901";
	char * wifiSSID = "Lacina";
	char * sendDataString = "Testing cubee";

	wanCtrl = WAN_Ctrl_getInstance();
	if(WAN_Ctrl_init(wanCtrl) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error initializing WAN");
	}

	/* update WAN configuration */
	memset(wanCtrl->wanControllerCfg->wifiPassword, 0, PASSWORD_MAX_SIZE);
	memset(wanCtrl->wanControllerCfg->wifiSSID, 0, SSID_MAX_SIZE);
	memmove(wanCtrl->wanControllerCfg->wifiPassword, wifiPassword, strlen(wifiPassword));
	memmove(wanCtrl->wanControllerCfg->wifiSSID, wifiSSID, strlen(wifiSSID));
	if(WAN_Ctrl_updateCfg(wanCtrl) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating WAN configuration");
	}

	for (;;)
	{
		ESP_LOGI(LOG_HEADER,"Running WAN Controller Test Application...\n");

		vTaskDelay(5000 / portTICK_PERIOD_MS);

		memmove(wanDataPacket.dataBuffer, (uint8_t *) sendDataString, strlen(sendDataString));
		wanDataPacket.dataType = WAN_CTRL_SND_DATA_COMMAND;
		wanDataPacket.dataBufferSize = strlen(sendDataString);
		WAN_Ctrl_sendData(wanCtrl,&wanDataPacket);
	}
}
#endif /* TEST_WAN_CONTROLLER */


#ifdef TEST_SENSOR_MGR
void __sensorMgr_test_task()
{

	SensorMgr_t * sensorMgr = SensorMgr_getInstance();
	SensorMgr_init(sensorMgr);
	int16_t AverageValue = 0;
	int16_t PeakValue = 0;
	uint16_t stDevValue = 0;

	for (;;)
	{
		ESP_LOGI(LOG_HEADER,"Running Sensor Manager Test Application...\n");

		for(uint8_t sensorId = 0; sensorId < SENSOR_MGR_ID_MAX; sensorId ++)
		{
			SensorMgr_printSensorBuffer(sensorMgr, sensorId);
			SensorMgr_readAvrgValue(sensorMgr, sensorId, &AverageValue);
			SensorMgr_readPeakValue(sensorMgr, sensorId, &PeakValue);
			SensorMgr_readSTDEV(sensorMgr, sensorId, &stDevValue);
			SensorMgr_resetSensor(sensorMgr, sensorId);
		}

		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}
}
#endif /* TEST_SENSOR_MGR */

#ifdef TEST_I2C
void _I2C_Master_test_task()
{
	int16_t accelerometerX;
	int16_t accelerometerY;
	int16_t accelerometerZ;
	int16_t temperature;
	int16_t gyroscopeX;
	int16_t gyroscopeY;
	int16_t gyroscopeZ;


	ESP_LOGI(LOG_HEADER,"Running I2C Master Test Application...\n");

	I2C_Master_t * i2cMaster = I2C_Master_getInstance();
	I2C_Master_init(i2cMaster);

	for (;;)
	{
		vTaskDelay(500 / portTICK_PERIOD_MS);

		if(I2C_Master_testCommunication(i2cMaster,I2C_ID_ACCELEROMETER) == CUBEE_ERROR_OK)
		{
			if(I2C_Master_read(i2cMaster, I2C_ID_ACCELEROMETER, PWR_MGMT_1 ,&acelerometerRegistersBuffer[0], 1) == CUBEE_ERROR_OK)
			{
				if(acelerometerRegistersBuffer[0] == 0x40)
				{
					ESP_LOGI(LOG_HEADER,"MPU6050 Sleeping");
					I2C_Master_writeReg(i2cMaster, I2C_ID_ACCELEROMETER, PWR_MGMT_1, 0x00);
				}
				else
				{
					/* Read accelerometer registers*/
					I2C_Master_read(i2cMaster, I2C_ID_ACCELEROMETER,ACCEL_ZOUT, &acelerometerRegistersBuffer[4],2);

					accelerometerX = ((int16_t)((acelerometerRegistersBuffer[0] << 8) | acelerometerRegistersBuffer[1]));
					accelerometerY = ((int16_t)((acelerometerRegistersBuffer[2] << 8) | acelerometerRegistersBuffer[3]));
					accelerometerZ = ((int16_t)((acelerometerRegistersBuffer[4] << 8) | acelerometerRegistersBuffer[5]));
					temperature = ((int16_t)((acelerometerRegistersBuffer[6] << 8) | acelerometerRegistersBuffer[7]))/340 + 36;
					gyroscopeX = ((int16_t)((acelerometerRegistersBuffer[8] << 8) | acelerometerRegistersBuffer[9]));
					gyroscopeY = ((int16_t)((acelerometerRegistersBuffer[10] << 8) | acelerometerRegistersBuffer[11]));
					gyroscopeZ = ((int16_t)((acelerometerRegistersBuffer[12] << 8) | acelerometerRegistersBuffer[13]));

					ESP_LOGI(LOG_HEADER,"Accelerometer (X,Y,Z) = (%d,%d,%d),Temperature = %d,Gyroscope (X,Y,Z) = (%d,%d,%d).",accelerometerX, accelerometerY, accelerometerZ, temperature, gyroscopeX, gyroscopeY, gyroscopeZ);
				}
			}

		}
	}
}
#endif /* TEST_I2C */


#ifdef TEST_STORAGE_MGR
void _StrMgr_test_task(){

	ESP_LOGI(LOG_HEADER,"Running Storage Manager Test Application...\n");

	StorageMgr_t * storageMgr = StorageMgr_getInstance();
	StorageMgr_Init(storageMgr);

	/*
	 * Test configuration storage
	 */
	ESP_LOGI(LOG_HEADER, "\r\n-----------------Test configuration storage----------------\r\n");	/* Clean RAM current configuration */
	/* Update RAM current configuration */
	memmove(cubeeCfg.cubeeName,cubeeName,CUBEE_NAME_MAX_SIZE);
	memmove(cubeeCfg.idCubee,cubeeId,ID_CUBEE_MAX_SIZE);
	memmove(cubeeCfg.wifiPassword,password,PASSWORD_MAX_SIZE);
	memmove(cubeeCfg.wifiSSID,ssid,SSID_MAX_SIZE);

	StorageMgr_saveCubeeCfg(storageMgr, &cubeeCfg);
	StorageMgr_readCubeeCfg(storageMgr, &cubeeCfg);


	for (;;)
	{

		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}

}
#endif /* TEST_STORAGE_MGR */

#ifdef TEST_MEMORY_MGR
void _MemoryMgr_test_task(){

	ESP_LOGI(LOG_HEADER,"Running Storage Manager Test Application...\n");

	MemoryMgr_t * memoryMgr = MemoryMgr_getInstance();
	if( MemoryMgr_Init(memoryMgr) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Memory manager initialization failed");
		return;
	}

	MemoryMgr_dataPartition_reset(memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_ALARM);
	MemoryMgr_dataPartition_reset(memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE);
	MemoryMgr_NVS_reset(memoryMgr);

//	/* Pop all data in sample partition */
//	ESP_LOGI(LOG_HEADER, "-----POP DATA -----");
//	memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBuffer = rcvBuffer;
//	while(!MemoryMgr_dataPartition_isEmpty(memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE))
//	{
//		if(MemoryMgr_dataPartition_pop(memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE) != CUBEE_ERROR_OK)
//		{
//			ESP_LOGE(LOG_HEADER, "Error poping data from sample partition");
//		}
//	}
//
//
//	ESP_LOGI(LOG_HEADER, "-----PUSH DATA -----");
//	memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBuffer = sendBuffer;
//	memoryMgr->dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE].dataBufferSize = SEND_BUFFER_SIZE;
	for (;;)
	{
//		memset(sendBuffer,'A',SEND_BUFFER_SIZE);
//		if(MemoryMgr_dataPartition_push(memoryMgr,MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE) != CUBEE_ERROR_OK)
//		{
//			ESP_LOGE(LOG_HEADER, "Error pushing data to sample partition");
//		}

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

}
#endif /* TEST_MEMORY_MGR */

#ifdef TEST_ADC
void _ADC_test_task()
{

	ESP_LOGI(LOG_HEADER,"Running ADC test task\n");
	int32_t current_mA = 0;
	int32_t adcValue = 0;

	ADC_t * adc = ADC_getInstance();
	ADC_init(adc);

	for (;;)
	{
		adcValue = ADC_read(adc, ADC_CURRENT_SENSOR_ID);
		if(adcValue == -1)
		{
			ESP_LOGE(LOG_HEADER, "Error reading current sensor ADC");
		}
		else
		{
			current_mA = (30*(adcValue - 1876))/(3014 - 1876);
			ESP_LOGI(LOG_HEADER,"ADC channel %d, value: %d, current (mA): %d", ADC_CURRENT_SENSOR_ID, adcValue, current_mA);
		}

		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}
#endif /* TEST_ADC */

#ifdef TEST_SPI_MASTER
/*
This ISR is called when the handshake line goes high.
*/
static void IRAM_ATTR gpio_handshake_isr_handler(void* arg)
{
    //Sometimes due to interference or ringing or something, we get two irqs after eachother. This is solved by
    //looking at the time between interrupts and refusing any interrupt too close to another one.
    static uint32_t lasthandshaketime;
    uint32_t currtime=xthal_get_ccount();
    uint32_t diff=currtime-lasthandshaketime;
    if (diff<240000) return; //ignore everything <1ms after an earlier irq
    lasthandshaketime=currtime;

    //Give the semaphore.
    BaseType_t mustYield=false;
    xSemaphoreGiveFromISR(rdySem, &mustYield);
    if (mustYield) portYIELD_FROM_ISR();
}

void _SPI_Master_test_task()
{

	ESP_LOGI(LOG_HEADER,"Running SPI Master test task");

    //GPIO config for the handshake line.
    gpio_config_t io_conf=
    {
        .intr_type=GPIO_PIN_INTR_POSEDGE,
        .mode=GPIO_MODE_INPUT,
        .pull_up_en=1,
        .pin_bit_mask=(1<<GPIO_HANDSHAKE)
    };

    /* Semaphore to synchronize  master and slave */
    rdySem=xSemaphoreCreateBinary();

    //Set up handshake line interrupt.
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_set_intr_type(GPIO_HANDSHAKE, GPIO_PIN_INTR_POSEDGE);
    gpio_isr_handler_add(GPIO_HANDSHAKE, gpio_handshake_isr_handler, NULL);

	SPI_t * spiMaster = SPI_getInstance();
	if(SPI_init(spiMaster) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing SPI Master");
	}
	else
	{
		spiMaster->dataTransferBuffer = sendbuf;
		spiMaster->dataReceiveBuffer = recvbuf;
		spiMaster->dataTransferBufferSize = SPI_MASTER_BUFFER_SIZE;

		/* Transfer data to slave each 5 seconds*/
		for(;;)
		{
	        memset(recvbuf, 0, SPI_MASTER_BUFFER_SIZE);
	        memset(sendbuf, 0, SPI_MASTER_BUFFER_SIZE);
	        sprintf((char *)sendbuf,"ARE YOU MAD WITH ME?\r\n");

		    if(xSemaphoreTake(rdySem, portMAX_DELAY) == pdTRUE)
		    {
				if(SPI_transferData(spiMaster,SPI_SLAVE_ID_RFID) != CUBEE_ERROR_OK)
				{
					ESP_LOGE(LOG_HEADER,"Error sending data to SPI slave device");
				}
				ESP_LOGI(LOG_HEADER,"Data sent to SPI slave. Response: %s", recvbuf);
		    }

		}
	}
}
#endif /* TEST_SPI_MASTER */

#ifdef TEST_SPI_SLAVE
void _SPI_Slave_test_task()
{
	ESP_LOGE(LOG_HEADER,"Running SPI slave test...");

    //Configuration for the handshake line
    gpio_config_t io_conf={
        .intr_type=GPIO_INTR_DISABLE,
        .mode=GPIO_MODE_OUTPUT,
        .pin_bit_mask=(1<<GPIO_HANDSHAKE)
    };

    //Configure handshake line as output
    gpio_config(&io_conf);

    spi_slave_transaction_t transaction;
    memset(&transaction, 0, sizeof(transaction));

	SPI_Slave_t * spiSlaveRFID = SPI_Slave_getInstance(SPI_SLAVE_ID_RFID);
	if(SPI_Slave_init(spiSlaveRFID) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing SPI Slave");
	}
	else
	{
		for(;;)
		{

	        memset(recvbuf, 0, SPI_MASTER_BUFFER_SIZE);
	        memset(sendbuf, 0, SPI_MASTER_BUFFER_SIZE);
	        sprintf((char *)sendbuf,"I AM NOT MAD WITH YOU!\r\n");

	        transaction.length=8*SPI_MASTER_BUFFER_SIZE;
	        transaction.tx_buffer=sendbuf;
	        transaction.rx_buffer=recvbuf;
	        /* This call enables the SPI slave interface to send/receive to the sendbuf and recvbuf. The transaction is
	        initialized by the SPI master, however, so it will not actually happen until the master starts a hardware transaction
	        by pulling CS low and pulsing the clock etc. In this specific example, we use the handshake line, pulled up by the
	        .post_setup_cb callback that is called as soon as a transaction is ready, to let the master know it is free to transfer
	        data.
	        */
	        if(spi_slave_transmit(HSPI_HOST, &transaction, portMAX_DELAY))
	        {
	        	ESP_LOGE(LOG_HEADER,"Error transmitting data to SPI master, SPI_HOST: %d ", HSPI_HOST);
	        }

		    if(xSemaphoreTake(spiSlaveRFID->slaveSem, portMAX_DELAY) == pdTRUE)
		    {
		    	ESP_LOGI(LOG_HEADER,"Slave received data from SPI master: %s", (char*)recvbuf);

		    }
		}
	}
}
#endif /* TEST_SPI_SLAVE */

#ifdef TEST_RFID
void _RFID_test_task()
{
	RfidApp_t * rfid = RfidApp_getInstance();
	if(RfidApp_init(rfid) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "RFID APP initialization failed");
		return;
	}

	for (;;)
	{
		ESP_LOGI(LOG_HEADER,"Running RFID test task\n");
		vTaskDelay(60000 / portTICK_PERIOD_MS);
	}
}
#endif /* TEST_RFID */

#ifdef TEST_DB9
void _DB9_test_task()
{
	IOMgr_t * ioMgr = IOMgr_getInstance();
	if(IOMgr_init(ioMgr) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "IO Manager initialization failed");
		return;
	}

	StorageMgr_t * storageMgr = StorageMgr_getInstance();
	if(StorageMgr_Init(storageMgr) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Storage Mgr initialization failed");
		return;
	}

	DB9App_t * db9App = DB9App_getInstance();
	db9App->ioMgr = ioMgr;
	db9App->storageMgr = storageMgr;
	if(DB9App_init(db9App) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "DB9 APP initialization failed");
		return;
	}

//	/* Activate rule execution */
//	DB9App_setDB9RuleExecution(db9App, false);
//
//	/* Configure the rule */
//	memset(&db9Rule, 0, sizeof(DB9App_rule_t));
//	db9Rule.numberOfStates = 8;
//	db9Rule.ruleStates[0].timeDuration = 1;
//	db9Rule.ruleStates[0].value = 0x81;
//	db9Rule.ruleStates[1].timeDuration = 1;
//	db9Rule.ruleStates[1].value = 0x42;
//	db9Rule.ruleStates[2].timeDuration = 1;
//	db9Rule.ruleStates[2].value = 0x24;
//	db9Rule.ruleStates[3].timeDuration = 1;
//	db9Rule.ruleStates[3].value = 0x18;
//	db9Rule.ruleStates[4].timeDuration = 1;
//	db9Rule.ruleStates[4].value = 0x00;
//	db9Rule.ruleStates[5].timeDuration = 1;
//	db9Rule.ruleStates[5].value = 0x18;
//	db9Rule.ruleStates[6].timeDuration = 1;
//	db9Rule.ruleStates[6].value = 0x24;
//	db9Rule.ruleStates[7].timeDuration = 1;
//	db9Rule.ruleStates[7].value = 0x42;
//
//	vTaskDelay(5000 / portTICK_PERIOD_MS);
//
//	/* Send rule */
//	DB9App_sendDB9Rule(db9App,&db9Rule);
//
//	/* Set command */
//	memset(&db9App->db9Command, 0, sizeof(DB9App_ruleState_t));
//	memmove(&db9App->db9Command,&db9Rule.ruleStates[0],sizeof(DB9App_ruleState_t));
//	DB9App_sendSignal(db9App,DB9_APP_SIGNAL_COMMAND);
//	vTaskDelay(30000 / portTICK_PERIOD_MS);

	for (;;)
	{
		DB9App_setDB9RuleExecution(db9App, true);
		ESP_LOGI(LOG_HEADER,"Running DB9 test task\n");
		vTaskDelay(60000 / portTICK_PERIOD_MS);

	}
}
#endif /* TEST_DB9 */

#ifdef TEST_MQTT

static void __MQTTReceive_calback(MQTT_Network_Data_Packet_t * data)
{
	ESP_LOGI(LOG_HEADER, "MQTT Data received, Topic: %s ; Data: %.*s", data->topic, data->dataBufferSize, (char *)data->dataBuffer);
}

void __MQTT_test_task()
{
	char * wifiPassword =	"Lacina_21011901";
	char * wifiSSID = "Lacina";
	MQTT_Network_Data_Packet_t mqttDataPacket;
	char * publishTopic = "cubee/states";
	char * publishData = "{\"idCubee\":\"0\",\"state\":0}";
	uint16_t publishDataSize = strlen(publishData);
	char * subscribeTopic = "cubee/0/command";

	/* Set data packet */
	mqttDataPacket.dataBuffer = (uint8_t *)publishData;
	mqttDataPacket.dataBufferSize = publishDataSize;
	mqttDataPacket.topic = publishTopic;

	Wifi_Ctrl_t * wifi = Wifi_Ctrl_getInstance();
	if(Wifi_Ctrl_init(wifi) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error initializing Wifi");
		return;
	}

	MQTT_Network_t * mqttNetwork = MQTT_Network_getInstance();
	MQTT_Network_registerRcvCallback(mqttNetwork,__MQTTReceive_calback);
	/* Add subscription */
	mqttNetwork->subscriptionsTopics[0] = subscribeTopic;
	mqttNetwork->subscriptionCount = 1;

	if(MQTT_Network_init(mqttNetwork) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error initializing MQTT API");
		return;
	}

	/* Update Wifi configuration */
	memset(wifi->wifiControllerCfg->wifiSSID, 0, SSID_MAX_SIZE);
	memset(wifi->wifiControllerCfg->wifiPassword, 0, PASSWORD_MAX_SIZE);

	memmove(wifi->wifiControllerCfg->wifiSSID, wifiSSID, strlen(wifiSSID));
	memmove(wifi->wifiControllerCfg->wifiPassword, wifiPassword, strlen(wifiPassword));
	if(Wifi_Ctrl_updateCfg(wifi) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating Wifi configuration");
		return;
	}

	for (;;)
	{
		vTaskDelay(20000 / portTICK_PERIOD_MS);

		if(MQTT_Network_sendData(mqttNetwork, &mqttDataPacket) != CUBEE_ERROR_OK)
		{
			ESP_LOGE(LOG_HEADER, "Error sending data type over MQTT network");
		}
	}
}
#endif /* TEST_MQTT */

void main_task(){

	CubeeApp_t * CubeeApp = CubeeApp_getInstance();
	CubeeApp_init(CubeeApp);

	vTaskDelete(mainTask);
}

void app_main()
{
	nvs_flash_init();

	ESP_LOGI(LOG_HEADER,"CUBEE ENERGY APPLICATION\n");

    /* Create main_task */
	xTaskCreate(&main_task, "main_task", 4096, NULL, 2, mainTask);
}
