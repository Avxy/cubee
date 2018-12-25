/****************************************************
 *  Model File: C:\Users\Positivo\Google Drive\Projeto CUBEE (Pessoal)\CUBEE
 *Project.EAP
 *  Model Path: CUBEE Model::Firmware Implementation::Applications::FSM::FSM
 *
 *  Tuesday, May 9, 2017 - 2:44:56 PM
 ****************************************************/

#ifndef FSM_H_
#define FSM_H_

typedef enum
{
    TIME_NANOSECONDS,
    TIME_MICROSECONDS,
    TIME_MILLISECONDS,
    TIME_SECONDS,
    TIME_MINUTES,
    TIME_HOURS
} time_units;

typedef struct FSM_STATEStruct
{
    unsigned short activeSubState;
    unsigned long startTime;
} FSM_STATE;

uint64_t FSM_getTimeElapsedasd(FSM_STATE * state, time_units unit);

uint64_t FSM_getTimeElapsedFromISR(FSM_STATE * state, time_units unit);

void FSM_startTime(FSM_STATE * state);

void FSM_startTimeFromISR(FSM_STATE * state);

#endif /* #ifndef H_FSM */
