#ifndef SYNC_H
#define SYNC_H

#include <stdint.h>

/*----------------------------------------------------------
    Initialization
----------------------------------------------------------*/

void SYNC_Init(void);

/*----------------------------------------------------------
    Processing
----------------------------------------------------------*/

void SYNC_Update(void);

void SYNC_EXTI_Handler(void);

/*----------------------------------------------------------
    Status
----------------------------------------------------------*/

uint8_t SYNC_IsPresent(void);

uint32_t SYNC_GetHalfPeriodUs(void);

uint32_t SYNC_GetFrequency_x10(void);

uint32_t SYNC_GetCounter(void);

#endif
