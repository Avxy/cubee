#ifndef PAN_CONTROLLER_CFG_H_
#define PAN_CONTROLLER_CFG_H_

/*Standard libraries*/
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/* Modules dependencies */
#include "../../util/ErrorCode/CUBEE_ErrorCode.h"

typedef struct PANController_Cfg
{
	bool initialized;
} PANController_Cfg;

CubeeErrorCode_t PANController_Cfg_init(PANController_Cfg * this);
PANController_Cfg * PANController_Cfg_getInstance(void);

#endif /* PAN_CONTROLLER_CFG_H_ */
