/*
 * MemoryMgr.h
 *
 *  Created on: 22 de jun de 2017
 *      Author: UFCG
 */

#ifndef _MEMORY_MGR_H_
#define _MEMORY_MGR_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "esp_partition.h"
#include "wear_levelling.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "../../../main/ProjectCfg.h"
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

#define	PARTITION_SIZE						(512*1024) /* 512 kB */
#define SAMPLE_SIZE							sizeof(MemoryMgr_Sample_t)
#define MAX_SAMPLE_NUMBER					(PARTITION_SIZE / SAMPLE_SIZE)

#define ALARM_SIZE							sizeof(MemoryMgr_Alarm_t)
#define MAX_ALARM_NUMBER					(PARTITION_SIZE / ALARM_SIZE)

#define	MEMORY_MGR_DATA_BUFFER_MAX_SIZE	(1024)									/*!< Specifies the maximum size allowed to read/write data in flash partition */



/**
 * *****************************************************************************************************************************************
 * NEW IMPLEMENTATION
 * *****************************************************************************************************************************************
 */

/*!< Data types stored in flash partitions */
typedef enum
{
	MEMORY_MGR_PARTITION_DATA_TYPE_ALARM = 0,
	MEMORY_MGR_PARTITION_DATA_TYPE_SENSOR_SAMPLE,

	MEMORY_MGR_PARTITION_DATA_TYPE_MAX,
}MemoryMgr_FlashPartition_dataType_t;

/*!< Data types stored in NVS flash */
typedef enum
{
	MEMORY_MGR_NVS_DATA_TYPE_CFG = 0,
	MEMORY_MGR_NVS_DATA_TYPE_DB9,

	MEMORY_MGR_NVS_DATA_TYPE_MAX,
}MemoryMgr_NVS_dataType_t;

typedef struct
{
	uint32_t header;
	uint32_t tail;
}MemoryMgr_ringBuffer_Cfg_t;

typedef struct
{
	MemoryMgr_FlashPartition_dataType_t dataType;			/*!< MemoryMgr data type of data stored in flash partition*/
	char ringBuffercfgTag[20];								/*!< Tag to store ring buffer configuration in NVS memory */
	MemoryMgr_ringBuffer_Cfg_t ringBufferCfg;				/*!< Keeps the current address of ring buffer header and tail implemented for this data partition */
	uint8_t * dataBuffer;									/*!< Buffer used to read/write data in flash partition*/
	uint16_t dataBufferSize;								/*!< Size of data stored in data buffer to read/write in flash partition*/
	wl_handle_t wlPartitionHandler;							/*!< Wear levelling Handler used to access data stored in flash partitions */
	esp_partition_t wlPartition;							/*!< ESP partition information*/
}MemoryMgr_partition_info_t;

typedef struct
{
	char nvsTag[20];										/*!< Tag to store this data type in NVS memory */
}MemoryMgr_NVS_data_info_t;

/*! Struct to instantiate a Storage Manager component */
typedef struct
{
	bool initialized;																/*!< Indicates if the WAN Controller component was initialized */
	nvs_handle nvsHandle;															/*!< Handler used to access data stored in NVS memory */
	MemoryMgr_partition_info_t dataPartition[MEMORY_MGR_PARTITION_DATA_TYPE_MAX];	/*!< Memory partitions used to store each data type */
	MemoryMgr_NVS_data_info_t  nvsDataInfo[MEMORY_MGR_NVS_DATA_TYPE_MAX];			/*!< Data types stored in NVS memory */
} MemoryMgr_t;
/**
 * *****************************************************************************************************************************************
 */

CubeeErrorCode_t MemoryMgr_Init(MemoryMgr_t * this);

/**
* @brief This function gets a MemoryMgr instance.
*
* @return pointer to a MemoryMgr component instance
*/
MemoryMgr_t * MemoryMgr_getInstance(void);

/**
 * @brief This function saving data to a existing field in NVS memory.
 *
 * @param this - pointer to module instance.
 * @param dataType - NVS specific data type.
 * @param data - pointer to data to be saved.
 * @param dataSize - size of data to be saved.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully saved in NVS memory.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t MemoryMgr_NVS_save(MemoryMgr_t* this, MemoryMgr_NVS_dataType_t dataType, uint8_t * data, uint32_t dataSize);

/**
 * @brief This function read the data stored in NVS memory.
 *
 * @param this - pointer to module instance.
 * @param dataType - NVS specific data type.
 * @param data - pointer to output buffer used to copy the NVS data read.
 * @param dataSize - size of data to be read.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully read from NVS memory.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t MemoryMgr_NVS_read(MemoryMgr_t* this, MemoryMgr_NVS_dataType_t dataType, uint8_t * data, uint32_t dataSize);

/**
 * @brief This function reset the data stored in NVS memory.
 *
 * @param this - pointer to module instance.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully reset.
 * @arg CUBEE_ERROR_INVALID_PARAMETER, if a invalid parameter was passed.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t MemoryMgr_NVS_reset(MemoryMgr_t* this);

/**
 * @brief This function push data to the memory partition allocated to a data type.
 *
 * The storage manager implements a circular buffer to push data into the memory partition. If the
 * available space isn't enough to store the data, the sector with oldest data will be erased and so
 * the newest data will be stored.
 *
 * The data buffer and data size to be pushed in the specific data type partition needs to be passed
 * using the parameters dataBuffer and dataBufferSize in the MemoryMgr instance.
 *
 * @param this - pointer to Storage Manager instance.
 * @param dataType - Storage Manager specific data type.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully pushed in the flash partition.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t MemoryMgr_dataPartition_push(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType);


/**
 * @brief This function remove data from the memory partition allocated to a specific data type.
 *
 * The storage manager implements a circular buffer to pop data from a specific memory partition.
 * Therefore, the data pushed will be the oldest data.
 *
 * The buffer and size to store data removed from specific flash partition shall be passed in the MemoryMgr parameters
 * dataBuffer and dataBufferSize .
 *
 * The size and data removed from the specific flash partition will be placed in the MemoryMgr parameters
 * dataBuffer and dataBufferSize .
 *
 * @param this - pointer to Storage Manager instance.
 * @param dataType - Storage Manager specific data type.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully popped from the flash partition.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t MemoryMgr_dataPartition_pop(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType);


/**
 * @brief This function read data from the memory partition allocated to a specific data type.
 *
 * The storage manager implements a circular buffer to read data from a specific memory partition.
 * Therefore, the data read will be the oldest data.
 *
 * The buffer and size to store data read from specific flash partition shall be passed in the MemoryMgr parameters
 * dataBuffer and dataBufferSize .
 *
 * The size and data read from the specific flash partition will be placed in the MemoryMgr parameters
 * dataBuffer and dataBufferSize .
 *
 * @param this - pointer to Storage Manager instance.
 * @param dataType - Storage Manager specific data type.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the data was successfully read from the flash partition.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t MemoryMgr_dataPartition_read(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType);

/**
 * @brief This function reset the memory partition allocated to a specific data type.
 *
 * @param this - pointer to Storage Manager instance.
 * @param dataType - Storage Manager specific data type.
 *
 * @return
 * @arg CUBEE_ERROR_OK, if the memory partition was successfully erased.
 * @arg CUBEE_ERROR_CODE, otherwise.
*/
CubeeErrorCode_t MemoryMgr_dataPartition_reset(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType);

/**
 * @brief This function verifies if a memory partition allocated to a specific data type is empty.
 *
 * @param this - pointer to Storage Manager instance.
 * @param dataType - Storage Manager specific data type.
 *
 * @return
 * @arg true, if the memory partition is empty.
 * @arg false, otherwise.
*/
bool MemoryMgr_dataPartition_isEmpty(MemoryMgr_t* this, MemoryMgr_FlashPartition_dataType_t dataType);

/**
 * @brief This debug function prints a buffer of bytes with a specified format.
 *
 * @param buffer - pointer to the byte buffer.
 * @param bufferSize - size of byte buffer.
 * @param columnSize size of columns used on printing format
 *
 * @return
*/
void MemoryMgr_printBytes(uint8_t* buffer, uint16_t bufferSize, uint16_t columnSize);

#endif /* _MEMORY_MGR_H_ */
