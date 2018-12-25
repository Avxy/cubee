/*
 * MemoryMgr.c
 *
 *  Created on: 22 de jun de 2017
 *      Author: UFCG
 */

#include "MemoryMgr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"


#define LOG_HEADER 					"[MEMORY MGR]"
#define DEBUG_MEMORY_MGR

// max length
#define MAX_SIZE 		20

#define NVS_DATA_TAG_CUBEE_CONFIG	"TAG_CUBEE_CFG"
#define NVS_DATA_TAG_DB9_RULE		"TAG_DB9_RULE"


#define RING_BUFFER_CFG_TAG_ALARM	"CFG_TAG_ALARM"
#define RING_BUFFER_CFG_TAG_SENSOR	"CFG_TAG_SENSOR"

#define SAMPLE_PARTITION_FLASH_ADDRESS					0x300000
#define ALARM_PARTITION_FLASH_ADDRESS					SAMPLE_PARTITION_FLASH_ADDRESS + PARTITION_SIZE
#define INITIAL_PARTITION_ADDRESS						0x00
#define FINAL_PARTITION_ADDRESS							(PARTITION_SIZE - 1) /*Relative address*/



/************************************************************************************************/
 /* Local Functions Definition */
 /************************************************************************************************/

static CubeeErrorCode_t __MemoryMgr_InitMemoryPartitions(MemoryMgr_t * this);
static CubeeErrorCode_t __MemoryMgr_InitConfigMemory(MemoryMgr_t * this);

/************************************************************************************************/
 /* Local Variables */
 /************************************************************************************************/
static MemoryMgr_t memoryMgrInstance =
{
		.initialized = false,
		.nvsHandle = NULL,
		.dataPartition =
		{
				[MEMORY_MGR_PARTITION_DATA_TYPE_ALARM] =
				{
						.dataType = MEMORY_MGR_PARTITION_DATA_TYPE_ALARM,
						.ringBufferCfg =
						{
								.tail = 0,
								.header = 0

						},
						.ringBuffercfgTag = RING_BUFFER_CFG_TAG_ALARM,
						.dataBuffer = NULL,
						.dataBufferSize = 0,
						.wlPartitionHandler = NULL,
						.wlPartition =
						{
							    .type = ESP_PARTITION_TYPE_DATA,       				/*!< partition type (app/data) */
							    .subtype = ESP_PARTITION_SUBTYPE_ANY,				/*!< partition subtype */
							    .address = ALARM_PARTITION_FLASH_ADDRESS,        	/*!< starting address of the partition in flash */
							    .size = PARTITION_SIZE,    							/*!< size of the partition, in bytes (1Mb)*/
							    .encrypted = false,                     			/*!< flag is set to true if partition is encrypted */
						},
				},
				[MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE] =
				{
						.dataType = MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE,
						.ringBufferCfg =
						{
								.tail = 0,
								.header = 0

						},
						.ringBuffercfgTag = RING_BUFFER_CFG_TAG_SENSOR,
						.dataBuffer = NULL,
						.dataBufferSize = 0,
						.wlPartitionHandler = NULL,
						.wlPartition =
						{
							    .type = ESP_PARTITION_TYPE_DATA,       				/*!< partition type (app/data) */
							    .subtype = ESP_PARTITION_SUBTYPE_ANY,				/*!< partition subtype */
							    .address =SAMPLE_PARTITION_FLASH_ADDRESS,        	/*!< starting address of the partition in flash */
							    .size = PARTITION_SIZE,    							/*!< size of the partition, in bytes (1Mb)*/
							    .encrypted = false,                     			/*!< flag is set to true if partition is encrypted */
						},
				},
		},
		.nvsDataInfo =
		{
				[MEMORY_MGR_NVS_DATA_TYPE_CFG] =
				{
						.nvsTag = NVS_DATA_TAG_CUBEE_CONFIG,
				},
				[MEMORY_MGR_NVS_DATA_TYPE_DB9] =
				{
						.nvsTag = NVS_DATA_TAG_DB9_RULE,
				},
		},

};

static uint32_t ringBufferCfgSize = sizeof(MemoryMgr_ringBuffer_Cfg_t);

/************************************************************************************************/
/* Public Functions Implementation */
/************************************************************************************************/

CubeeErrorCode_t MemoryMgr_Init(MemoryMgr_t * this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER, "CUBEE_ERROR_INVALID_PARAMETER initializing Memory Manager driver");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}
	else if(this->initialized != false)
	{
		/* Already initialized */
		return CUBEE_ERROR_OK;
	}

	ESP_LOGI(LOG_HEADER,"Initializing Memory Manager driver ...");

	if(__MemoryMgr_InitConfigMemory(this) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing configuration memory");
		return CUBEE_ERROR_UNDEFINED;
	}

	if(__MemoryMgr_InitMemoryPartitions(this) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error initializing memory partitions");
		return CUBEE_ERROR_UNDEFINED;
	}

	this->initialized = true;

    ESP_LOGI(LOG_HEADER,"Memory Manager successfully initialized");

	return CUBEE_ERROR_OK;
}

MemoryMgr_t * MemoryMgr_getInstance(void)
{
	return &memoryMgrInstance;
}

CubeeErrorCode_t MemoryMgr_NVS_save(MemoryMgr_t* this, MemoryMgr_NVS_dataType_t dataType, uint8_t * data, uint32_t dataSize)
{
	esp_err_t err;

	if((this == NULL) || (data == NULL) || (dataType >= MEMORY_MGR_NVS_DATA_TYPE_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER saving data to NVS memory");
		return CUBEE_ERROR_INVALID_PARAMETER;

	}

	err = nvs_set_blob(this->nvsHandle, this->nvsDataInfo[dataType].nvsTag, data, dataSize);
	if(err != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error saving data in NVS memory, data type: %d, ESP Error Code: (%04X)", dataType, err);
		return CUBEE_ERROR_UNDEFINED;
	}
	/* Sending modifications to NVS flash*/
	err = nvs_commit(this->nvsHandle);
	if(err != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error committing NVS modifications, data type: %d, (%04X)", dataType, err);
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef DEBUG_MEMORY_MGR
	ESP_LOGI(LOG_HEADER,"Data successfully saved to NVS memory, NVS data type: %d", dataType);
#endif /* DEBUG_MEMORY_MGR */

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t MemoryMgr_NVS_read(MemoryMgr_t* this, MemoryMgr_NVS_dataType_t dataType, uint8_t * data, uint32_t dataSize)
{
	esp_err_t err;

	if((this == NULL) || (data == NULL) || (dataType >= MEMORY_MGR_NVS_DATA_TYPE_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading data from NVS memory");
		return CUBEE_ERROR_INVALID_PARAMETER;

	}

	/* Clean output buffer */
	memset(data,0,dataSize);

	err = nvs_get_blob(this->nvsHandle, this->nvsDataInfo[dataType].nvsTag, data, &dataSize);
	if(err != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error reading data from NVS memory, data type: %d, ESP Error Code: (%04X)", dataType, err);
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef DEBUG_MEMORY_MGR
	ESP_LOGI(LOG_HEADER,"Data successfully read from NVS memory, NVS data type: %d", dataType);
#endif /* DEBUG_MEMORY_MGR */

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t MemoryMgr_NVS_reset(MemoryMgr_t* this)
{
	if(this == NULL)
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reseting NVS memory");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(nvs_erase_all(this->nvsHandle) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error reseting NVS memory");
		return CUBEE_ERROR_UNDEFINED;
	}

	if(nvs_commit(this->nvsHandle))
	{
		ESP_LOGE(LOG_HEADER,"Error committing NVS modifications");
		return CUBEE_ERROR_UNDEFINED;
	}

#ifdef DEBUG_MEMORY_MGR
	ESP_LOGI(LOG_HEADER,"NVS memory successfully reset");
#endif /* DEBUG_MEMORY_MGR */

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t MemoryMgr_dataPartition_push(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType)
{

	uint32_t newHeader;
	uint16_t newHeaderSector;
	uint16_t headerSector;
	uint16_t tailSector;
	uint16_t sectorSize;
	uint16_t sectorMax;
	uint16_t partitionSize;
	uint32_t firstStoreSize;
	uint32_t secondStoreSize;

	if((this == NULL) || (!this->initialized) || (dataType >= MEMORY_MGR_PARTITION_DATA_TYPE_MAX) ||
			(this->dataPartition[dataType].dataBuffer == NULL) || (this->dataPartition[dataType].dataBufferSize == 0)
			|| (this->dataPartition[dataType].dataBufferSize > wl_sector_size(this->dataPartition[dataType].wlPartitionHandler)))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER pushing data to flash memory partition");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	sectorSize = wl_sector_size(this->dataPartition[dataType].wlPartitionHandler);
	partitionSize = wl_size(this->dataPartition[dataType].wlPartitionHandler);
	sectorMax = (partitionSize / sectorSize);


	/* Calculate headers and sectors */
	newHeader = (this->dataPartition[dataType].ringBufferCfg.header + this->dataPartition[dataType].dataBufferSize) % partitionSize;
	newHeaderSector = newHeader / sectorSize;
	headerSector = this->dataPartition[dataType].ringBufferCfg.header / sectorSize;
	tailSector = this->dataPartition[dataType].ringBufferCfg.tail / sectorSize;

#ifdef DEBUG_MEMORY_MGR
	ESP_LOGI(LOG_HEADER, "Pushing data type %d, sector size: %d, partition size: %d, data size: %d", dataType, sectorSize, partitionSize, this->dataPartition[dataType].dataBufferSize);
	ESP_LOGI(LOG_HEADER, "tail address: %d, tail sector: %d header address: %d, header sector: %d, new header address: %d, new header sector: %d \r\n",
				this->dataPartition[dataType].ringBufferCfg.tail, tailSector, this->dataPartition[dataType].ringBufferCfg.header, headerSector,	newHeader, newHeaderSector);
#endif /* DEBUG_MEMORY_MGR */

	/* clean new header sector on sector transition or first push */
	if((newHeaderSector != headerSector) ||
			((this->dataPartition[dataType].ringBufferCfg.header == INITIAL_PARTITION_ADDRESS) && (this->dataPartition[dataType].ringBufferCfg.tail == INITIAL_PARTITION_ADDRESS)))
	{
		if(wl_erase_range(this->dataPartition[dataType].wlPartitionHandler, newHeaderSector * sectorSize, sectorSize) != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error pushing data type %d, failed erasing sector %d", dataType, newHeaderSector);
			return CUBEE_ERROR_UNDEFINED;
		}

#ifdef DEBUG_MEMORY_MGR
		ESP_LOGI(LOG_HEADER,"Pushing data type %d, sector transition identified, new sector %d successfully erased", dataType, newHeaderSector);
#endif /* DEBUG_MEMORY_MGR */
	}

	/* Verify sector transition */
	if(newHeaderSector != headerSector)
	{
		/* Verify if the new header sector reached the tail sector, if so update the tail sector. In that implementation, the newest data has preference over the oldest data */
		if(newHeaderSector == tailSector)
		{
			/* Shift the tail to the begin of next sector */
			tailSector = ((tailSector + 1) % sectorMax);
			this->dataPartition[dataType].ringBufferCfg.tail = tailSector * sectorSize;
			/* Update tail in NVS memory */
			if(nvs_set_blob(this->nvsHandle, this->dataPartition[dataType].ringBuffercfgTag, &this->dataPartition[dataType].ringBufferCfg,ringBufferCfgSize) != ESP_OK)
			{
				ESP_LOGE(LOG_HEADER, "Error updating tail to data type %d.", dataType);
			}
#ifdef DEBUG_MEMORY_MGR
		ESP_LOGI(LOG_HEADER,"Pushing data type %d, partition is full, tail sector %d was erased and updated to %d", dataType, newHeaderSector, tailSector);
#endif /* DEBUG_MEMORY_MGR */
		}

		/* Verifies bottom-up transition */
		if(newHeaderSector < headerSector)
		{
			/* Split storage in two steps . Write first part: size = partition size - current stored size */
			firstStoreSize = partitionSize - (this->dataPartition[dataType].ringBufferCfg.header);
			if(wl_write(this->dataPartition[dataType].wlPartitionHandler, this->dataPartition[dataType].ringBufferCfg.header, this->dataPartition[dataType].dataBuffer, firstStoreSize) != ESP_OK)
			{
				ESP_LOGE(LOG_HEADER,"Error pushing data to flash memory partition, failed storing first part of the data splitting");
				return CUBEE_ERROR_UNDEFINED;
			}
#ifdef DEBUG_MEMORY_MGR
		ESP_LOGI(LOG_HEADER,"Pushing data type %d, bottom-up transition identified, first store executed, data size: %d", dataType, firstStoreSize);
#endif /* DEBUG_MEMORY_MGR */
			/* Split storage in two steps. Write second part: size = data size - (partition size - current stored size) */
			secondStoreSize = this->dataPartition[dataType].dataBufferSize - firstStoreSize;
			if(secondStoreSize > 0)
			{
				if(wl_write(this->dataPartition[dataType].wlPartitionHandler, INITIAL_PARTITION_ADDRESS, &this->dataPartition[dataType].dataBuffer[firstStoreSize] , secondStoreSize)  != ESP_OK)
				{
					ESP_LOGE(LOG_HEADER,"Error pushing data to flash memory partition, failed storing second part of the data splitting");
					return CUBEE_ERROR_UNDEFINED;
				}
#ifdef DEBUG_MEMORY_MGR
		ESP_LOGI(LOG_HEADER,"Pushing data type %d, bottom-up transition identified, second store executed, data size: %d", dataType, secondStoreSize);
#endif /* DEBUG_MEMORY_MGR */
			}
		}
	}

	/* Standard storage */
	if((newHeaderSector >= headerSector))
	{
		/* Standard storage */
		if(wl_write(this->dataPartition[dataType].wlPartitionHandler, this->dataPartition[dataType].ringBufferCfg.header, this->dataPartition[dataType].dataBuffer , this->dataPartition[dataType].dataBufferSize)  != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error pushing data to flash memory partition, storage failed");
			return CUBEE_ERROR_UNDEFINED;
		}
	}

	/* Updates data partition information */
	this->dataPartition[dataType].ringBufferCfg.header = newHeader;
	/* Update header in NVS memory */
	if(nvs_set_blob(this->nvsHandle, this->dataPartition[dataType].ringBuffercfgTag, &this->dataPartition[dataType].ringBufferCfg ,ringBufferCfgSize) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating header to data type %d.", dataType);
	}
	ESP_LOGI(LOG_HEADER,"Data type %d successfully pushed, tail address: %d, header address: %d",dataType, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].ringBufferCfg.header);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t MemoryMgr_dataPartition_pop(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType)
{

	if((this == NULL) || (!this->initialized) || (dataType >= MEMORY_MGR_PARTITION_DATA_TYPE_MAX) || (this->dataPartition[dataType].dataBuffer == NULL) || (this->dataPartition[dataType].dataBufferSize == 0))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER popping data from flash memory partition");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(MemoryMgr_dataPartition_read(this, dataType) != CUBEE_ERROR_OK)
	{
		ESP_LOGE(LOG_HEADER,"Pop data type %d from flash memory failed", dataType)
		return CUBEE_ERROR_UNDEFINED;
	}

	/* Update tail */
	this->dataPartition[dataType].ringBufferCfg.tail = (this->dataPartition[dataType].ringBufferCfg.tail + this->dataPartition[dataType].dataBufferSize) % this->dataPartition[dataType].wlPartition.size;
	/* Update tail in NVS memory */
	if(nvs_set_blob(this->nvsHandle, this->dataPartition[dataType].ringBuffercfgTag, &this->dataPartition[dataType].ringBufferCfg ,ringBufferCfgSize) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating tail to data type %d.", dataType);
	}

	ESP_LOGI(LOG_HEADER,"Successfully pop Data type %d from flash partition , tail address: %d, header address: %d",dataType, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].ringBufferCfg.header);

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t MemoryMgr_dataPartition_read(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType)
{
	uint32_t firstReadize;
	uint32_t secondReadSize;

	if((this == NULL) || (!this->initialized) || (dataType >= MEMORY_MGR_PARTITION_DATA_TYPE_MAX) || (this->dataPartition[dataType].dataBuffer == NULL) || (this->dataPartition[dataType].dataBufferSize == 0))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reading data from flash memory partition");
		ESP_LOGE(LOG_HEADER, "(this->dataPartition[dataType].dataBuffer == NULL) - %d; (this->dataPartition[dataType].dataBufferSize == 0) - %d",
				this->dataPartition[dataType].dataBuffer == NULL, this->dataPartition[dataType].dataBufferSize == 0);
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	/* Clean read buffer */
	memset(this->dataPartition[dataType].dataBuffer,0,this->dataPartition[dataType].dataBufferSize);

	if( this->dataPartition[dataType].ringBufferCfg.tail ==  this->dataPartition[dataType].ringBufferCfg.header)
	{
		/* Data partition empty */
		this->dataPartition[dataType].dataBufferSize = 0;
		ESP_LOGI(LOG_HEADER,"Read data type %d from flash partition, the partition is empty, tail address: %d, header address: %d", dataType, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].ringBufferCfg.header)
	}
	else if(this->dataPartition[dataType].ringBufferCfg.tail <  this->dataPartition[dataType].ringBufferCfg.header)
	{
		/* The maximum data size read must be this->dataPartition[dataType].dataBufferSize */
		if((this->dataPartition[dataType].ringBufferCfg.header - this->dataPartition[dataType].ringBufferCfg.tail) <= this->dataPartition[dataType].dataBufferSize)
		{
			this->dataPartition[dataType].dataBufferSize = this->dataPartition[dataType].ringBufferCfg.header - this->dataPartition[dataType].ringBufferCfg.tail;
		}
		if (wl_read(this->dataPartition[dataType].wlPartitionHandler, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].dataBuffer, this->dataPartition[dataType].dataBufferSize) != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER,"Error to read data type %d from flash memory partition, failed reading data. Tail address: %d, header address: %d", dataType, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].ringBufferCfg.header );
			return CUBEE_ERROR_UNDEFINED;
		}

	}
	else if(this->dataPartition[dataType].ringBufferCfg.tail >  this->dataPartition[dataType].ringBufferCfg.header)
	{
		/* The maximum data size read must be MEMORY_MGR_DATA_BUFFER_MAX_SIZE */
		if((this->dataPartition[dataType].wlPartition.size - this->dataPartition[dataType].ringBufferCfg.tail) >= this->dataPartition[dataType].dataBufferSize)
		{
			if (wl_read(this->dataPartition[dataType].wlPartitionHandler, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].dataBuffer, this->dataPartition[dataType].dataBufferSize) != ESP_OK)
			{
				ESP_LOGE(LOG_HEADER,"Error to read data type %d from flash memory partition, failed reading data. Tail address: %d, header address: %d", dataType, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].ringBufferCfg.header );
				return CUBEE_ERROR_UNDEFINED;
			}
		}
		else
		{
			/* Split the reading in two steps */
			firstReadize = this->dataPartition[dataType].wlPartition.size - this->dataPartition[dataType].ringBufferCfg.tail;
			if (wl_read(this->dataPartition[dataType].wlPartitionHandler, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].dataBuffer, firstReadize) != ESP_OK)
			{
				ESP_LOGE(LOG_HEADER,"Error to read data type %d from flash memory partition, failed reading data. Tail address: %d, header address: %d", dataType, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].ringBufferCfg.header );
				return CUBEE_ERROR_UNDEFINED;
			}

			/* The space remaining i the buffer is (MEMORY_MGR_DATA_BUFFER_MAX_SIZE - firstReadize)  */
			secondReadSize = this->dataPartition[dataType].ringBufferCfg.header <= (this->dataPartition[dataType].dataBufferSize - firstReadize) ?
								this->dataPartition[dataType].ringBufferCfg.header: (this->dataPartition[dataType].dataBufferSize - firstReadize);
			if(secondReadSize > 0)
			{
				if (wl_read(this->dataPartition[dataType].wlPartitionHandler, INITIAL_PARTITION_ADDRESS, &this->dataPartition[dataType].dataBuffer[firstReadize], secondReadSize) != ESP_OK)
				{
					ESP_LOGE(LOG_HEADER,"Error to read data type %d from flash memory partition, failed reading data. Tail address: %d, header address: %d", dataType, this->dataPartition[dataType].ringBufferCfg.tail, this->dataPartition[dataType].ringBufferCfg.header );
					return CUBEE_ERROR_UNDEFINED;
				}
			}
			this->dataPartition[dataType].dataBufferSize = firstReadize + secondReadSize;
		}
	}

	return CUBEE_ERROR_OK;
}

CubeeErrorCode_t MemoryMgr_dataPartition_reset(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType)
{
	if((this == NULL) || (!this->initialized) || (dataType >= MEMORY_MGR_PARTITION_DATA_TYPE_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER reseting memory partition");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	if(wl_erase_range(this->dataPartition[dataType].wlPartitionHandler, INITIAL_PARTITION_ADDRESS, this->dataPartition[dataType].wlPartition.size) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER,"Error erasing data partition for data type %d", dataType);
		return CUBEE_ERROR_UNDEFINED;
	}

	this->dataPartition[dataType].ringBufferCfg.header = INITIAL_PARTITION_ADDRESS;
	this->dataPartition[dataType].ringBufferCfg.tail = INITIAL_PARTITION_ADDRESS;

	/* Update tail and header in NVS memory*/
	if(nvs_set_blob(this->nvsHandle, this->dataPartition[dataType].ringBuffercfgTag, &this->dataPartition[dataType].ringBufferCfg ,ringBufferCfgSize) != ESP_OK)
	{
		ESP_LOGE(LOG_HEADER, "Error updating tail and header to data type %d.", dataType);
		return CUBEE_ERROR_UNDEFINED;
	}

	ESP_LOGI(LOG_HEADER,"Memory partition for data type %d successfully reset",dataType);
	return CUBEE_ERROR_OK;
}

bool MemoryMgr_dataPartition_isEmpty(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType)
{
	if((this == NULL) || (!this->initialized) || (dataType >= MEMORY_MGR_PARTITION_DATA_TYPE_MAX))
	{
		ESP_LOGE(LOG_HEADER,"CUBEE_ERROR_INVALID_PARAMETER verifying if the flash partition is empty");
		return CUBEE_ERROR_INVALID_PARAMETER;
	}

	return (this->dataPartition[dataType].ringBufferCfg.tail ==  this->dataPartition[dataType].ringBufferCfg.header);
}


/************************************************************************************************/
/* Local Functions Implementation */
/************************************************************************************************/

static CubeeErrorCode_t __MemoryMgr_InitConfigMemory(MemoryMgr_t * this)
{

	const esp_partition_t* nvs_partition;

	ESP_LOGI(LOG_HEADER,"Initializing configuration memory ...");

    if (nvs_flash_init() == ESP_ERR_NVS_NO_FREE_PAGES)
    {
    	/* if it is invalid, try to erase it */
    	ESP_LOGE(LOG_HEADER,"Got NO_FREE_PAGES error, trying to erase the partition...");

		/*  find the NVS partition */
        nvs_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, NULL);

        if(!nvs_partition)
		{
        	ESP_LOGE(LOG_HEADER,"FATAL ERROR: No NVS partition found");
        	return CUBEE_ERROR_UNDEFINED;
		}

		/* erase the partition */
		if(esp_partition_erase_range(nvs_partition, 0, nvs_partition->size) != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER,"FATAL ERROR: Unable to erase the partition");
			return CUBEE_ERROR_UNDEFINED;
		}
		ESP_LOGI(LOG_HEADER,"Partition erased!");

		if(nvs_flash_init() != ESP_OK)
		{
			ESP_LOGE(LOG_HEADER,"FATAL ERROR: Unable to initialize NVS");
			return CUBEE_ERROR_UNDEFINED;
		}
	}

	/* open the partition in Read/Write mode */
    if (nvs_open("storage", NVS_READWRITE, &this->nvsHandle) != ESP_OK)
    {
    	ESP_LOGE(LOG_HEADER,"FATAL ERROR: Unable to open NVS");
		return CUBEE_ERROR_UNDEFINED;
	}

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    /* Initialize tail and header for all data partitions */
    for(uint8_t dataPartitionId = 0; dataPartitionId <  MEMORY_MGR_PARTITION_DATA_TYPE_MAX; dataPartitionId++)
    {
    	if(nvs_get_blob(this->nvsHandle, this->dataPartition[dataPartitionId].ringBuffercfgTag, &this->dataPartition[dataPartitionId].ringBufferCfg, &ringBufferCfgSize) != ESP_OK)
    	{
    		ESP_LOGE(LOG_HEADER,"Error reading tail from NVS memory, data partition type: %d", dataPartitionId);
    	}
    }

    ESP_LOGI(LOG_HEADER,"Configuration memory successfully initialized");

	return CUBEE_ERROR_OK;
}

static CubeeErrorCode_t __MemoryMgr_InitMemoryPartitions(MemoryMgr_t * this)
{
	ESP_LOGI(LOG_HEADER,"Initializing memory partitions...");

    /* Initialize tail and header for all data partitions */
    for(uint8_t dataPartitionId = 0; dataPartitionId <  MEMORY_MGR_PARTITION_DATA_TYPE_MAX; dataPartitionId++)
    {
    	if (wl_mount(&this->dataPartition[dataPartitionId].wlPartition,
    			&this->dataPartition[dataPartitionId].wlPartitionHandler) != ESP_OK)
    	{
    		ESP_LOGE(LOG_HEADER,"Error mounting flash partition, data partition type: %d", dataPartitionId);
    		return CUBEE_ERROR_UNDEFINED;
    	}
    }

	ESP_LOGI(LOG_HEADER,"Memory partitions successfully initialized");

	return CUBEE_ERROR_OK;
}

void MemoryMgr_printBytes(uint8_t* buffer, uint16_t bufferSize, uint16_t columnSize)
{
    int index;

    printf("Print buffer, size: %d\r\n", bufferSize);
    for (index = 0; index < bufferSize; index++)
    {
        printf("%#02x ", buffer[index]);
        if (( index + 1 ) % columnSize == 0)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
}


