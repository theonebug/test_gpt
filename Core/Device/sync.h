#ifndef SYNC_H
#define SYNC_H

#include "main.h"

/*----------------------------------------------------------
    Public functions
----------------------------------------------------------*/

void SYNC_Init(void);
void SYNC_EXTI_Handler(void);
void SYNC_Update(void);

uint8_t SYNC_IsPresent(void);
uint32_t SYNC_GetHalfPeriodUs(void);
uint32_t SYNC_GetCounter(void);
uint32_t SYNC_GetFrequency_x10(void);
void SYNC_ResetCounter(void);

uint32_t SYNC_TestData(void);

#endif /* SYNC_H */
