#ifndef TIRISTOR_H
#define TIRISTOR_H

#include <stdint.h>

typedef enum
{
    TIRISTOR_MODE_FIRST = 0,
    TIRISTOR_MODE_ALL,
    TIRISTOR_MODE_FULL_ON

} TiristorMode_t;

typedef enum
{
    TIRISTOR_IDLE = 0,
    TIRISTOR_WAIT_FIRST,
    TIRISTOR_PULSE,
    TIRISTOR_CONDUCTION,
    TIRISTOR_FINISHED

} TiristorState_t;

void Tiristor_Init(void);

void Tiristor_Update(void);

void Tiristor_Start(void);

void Tiristor_Stop(void);

uint8_t Tiristor_IsActive(void);

void Tiristor_SetMode(TiristorMode_t mode);

TiristorMode_t Tiristor_GetMode(void);

void Tiristor_OnZeroCross(void);

uint32_t Tiristor_GetDelay(void);

#endif
