/*
 * sync_stub.c - хост-заглушка модуля синхронизации (Core/Device/sync.c).
 *
 * Реальную логику измерения полупериода по EXTI на тестах не воспроизводим:
 * тест сам задаёт величину полупериода и наличие синхронизации через
 * управляющие функции внизу этого файла.
 */

#include "sync.h"
#include "main.h"
#include "device.h"

static uint8_t  StubSyncPresent = 0U;
static uint32_t StubHalfPeriodUs = 0U;
static uint32_t StubCounter = 0U;
static uint32_t StubGlitch = 0U;
static uint32_t StubFreqX10 = 0U;

/*--- Управляющие функции для тестов ---*/
void HOST_SYNC_SetPresent(uint8_t p)     { StubSyncPresent = (p != 0U); }
void HOST_SYNC_SetHalfPeriodUs(uint32_t hp) { StubHalfPeriodUs = hp;
    StubFreqX10 = (hp == 0U) ? 0U : (5000000UL / hp); }
void HOST_SYNC_ResetCounter(void)        { StubCounter = 0U; StubGlitch = 0U; }

/*--- API синхронизации (реализации-заглушки) ---*/
void SYNC_Init(void)     { StubCounter = 0U; StubGlitch = 0U; }
void SYNC_Update(void)   { (void)0; }
void SYNC_EXTI_Handler(void) { StubCounter++; }

uint8_t  SYNC_IsPresent(void)      { return StubSyncPresent; }
uint32_t SYNC_GetHalfPeriodUs(void){ return StubHalfPeriodUs; }
uint32_t SYNC_GetCounter(void)     { return StubCounter; }
uint32_t SYNC_GetGlitchCounter(void){ return StubGlitch; }
uint32_t SYNC_GetFrequency_x10(void){ return StubFreqX10; }

void SYNC_SetFreqWindow(uint32_t minFreqX10, uint32_t maxFreqX10)
{
    (void)minFreqX10; (void)maxFreqX10;
}

uint32_t SYNC_TestData(void){ return 0U; }