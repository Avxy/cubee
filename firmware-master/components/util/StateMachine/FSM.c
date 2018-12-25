/****************************************************
 *  Model File: C:\Users\Positivo\Google Drive\Projeto CUBEE (Pessoal)\CUBEE
 *Project.EAP
 *  Model Path: CUBEE Model::Firmware Implementation::Applications::FSM::FSM
 *
 *  Tuesday, May 9, 2017 - 2:44:56 PM
 ****************************************************/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "FSM.h"


uint64_t FSM_getTimeElapsedasd(FSM_STATE * state, time_units unit)
{
	uint64_t timeElapsed_ms = (xTaskGetTickCount() - state->startTime) * portTICK_PERIOD_MS;
	uint64_t timeElapsed_unit = 0;

    switch (unit)
    {
    case TIME_NANOSECONDS:
    	timeElapsed_unit = timeElapsed_ms * 10e6;
        break;
    case TIME_MICROSECONDS:
    	timeElapsed_unit = timeElapsed_ms * 10e3;
        break;
    case TIME_MILLISECONDS:
    	timeElapsed_unit = timeElapsed_ms;
        break;
    case TIME_SECONDS:
    	timeElapsed_unit = timeElapsed_ms / 10e3;
        break;
    case TIME_MINUTES:
    	timeElapsed_unit = timeElapsed_ms / (60 * 10e3);
        break;
    case TIME_HOURS:
    	timeElapsed_unit = timeElapsed_ms / (60 * 60 * 10e3);
        break;
    default:
    	break;
    }

    return timeElapsed_unit;
}

uint64_t FSM_getTimeElapsedFromISR(FSM_STATE * state, time_units unit)
{
	uint64_t timeElapsed_ms = (xTaskGetTickCountFromISR() - state->startTime) * portTICK_PERIOD_MS;
	uint64_t timeElapsed_unit = 0;

    switch (unit)
    {
    case TIME_NANOSECONDS:
    	timeElapsed_unit = timeElapsed_ms * 10e6;
        break;
    case TIME_MICROSECONDS:
    	timeElapsed_unit = timeElapsed_ms * 10e3;
        break;
    case TIME_MILLISECONDS:
    	timeElapsed_unit = timeElapsed_ms;
        break;
    case TIME_SECONDS:
    	timeElapsed_unit = timeElapsed_ms / 10e3;
        break;
    case TIME_MINUTES:
    	timeElapsed_unit = timeElapsed_ms / (60 * 10e3);
        break;
    case TIME_HOURS:
    	timeElapsed_unit = timeElapsed_ms / (60 * 60 * 10e3);
        break;
    default:
    	break;
    }

    return timeElapsed_unit;
}

void FSM_startTime(FSM_STATE * state)
{
	state->startTime = xTaskGetTickCount();
}

void FSM_startTimeFromISR(FSM_STATE * state)
{
	state->startTime = xTaskGetTickCountFromISR();
}

