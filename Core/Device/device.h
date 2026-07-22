#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

/*----------------------------------------------------------
    Device state
----------------------------------------------------------*/

typedef struct
{
    uint16_t Angle;

    uint16_t PulseDuration;

    uint8_t SyncOK;

    uint8_t RS485OK;

    uint8_t Remote;

} Device_t;

extern Device_t Device;

/*----------------------------------------------------------
    Initialization
----------------------------------------------------------*/

void Device_Init(void);

/*----------------------------------------------------------
    Angle
----------------------------------------------------------*/

void Device_SetAngle(uint16_t angle);

uint16_t Device_GetAngle(void);

#endif
