#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>

/*----------------------------------------------------------
    Device state machine
----------------------------------------------------------*/

typedef enum
{
    DEVICE_READY = 0,

    DEVICE_WAIT_SYNC,

    DEVICE_WAIT_ZERO,

    DEVICE_TEST,

    DEVICE_FINISHED

} DeviceState_t;

/*----------------------------------------------------------
    Device parameters
----------------------------------------------------------*/

typedef struct
{
    uint16_t Angle;

    uint16_t PulseDuration;

    uint8_t SyncOK;

    uint8_t RS485OK;

    uint8_t Remote;

    DeviceState_t State;

} Device_t;

extern Device_t Device;

void Device_OnZeroCross(void);

/*----------------------------------------------------------
    Initialization
----------------------------------------------------------*/

void Device_Init(void);

/*----------------------------------------------------------
    Main state machine
----------------------------------------------------------*/

void Device_Start(void);

void Device_Stop(void);

void Device_Update(void);

uint32_t Device_GetZeroCrossCounter(void);

/* Время с начала текущего запуска, мс */
uint32_t Device_GetElapsedMs(void);

DeviceState_t Device_GetState(void);

/* 1 - устройство в READY: разрешены угол, меню и настройки */
uint8_t Device_IsIdle(void);

/*----------------------------------------------------------
    Angle
----------------------------------------------------------*/

void Device_SetAngle(uint16_t angle);

uint16_t Device_GetAngle(void);

#endif
