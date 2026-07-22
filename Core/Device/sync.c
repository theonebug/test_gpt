#include "sync.h"
#include "main.h"
#include "device.h"

extern TIM_HandleTypeDef htim2;

/*----------------------------------------------------------
    Variables
----------------------------------------------------------*/

static volatile uint32_t SyncCounter = 0;

static volatile uint32_t LastTime = 0;

static volatile uint32_t HalfPeriod = 0;

static volatile uint8_t SyncPresent = 0;

static volatile uint32_t LastPulseTick = 0;

/*----------------------------------------------------------
    Initialization
----------------------------------------------------------*/

void SYNC_Init(void)
{
    SyncCounter = 0;

    LastTime = __HAL_TIM_GET_COUNTER(&htim2);

    HalfPeriod = 0;

    SyncPresent = 0;

    LastPulseTick = HAL_GetTick();
}

/*----------------------------------------------------------
    EXTI callback
----------------------------------------------------------*/

void SYNC_EXTI_Handler(void)
{
    uint32_t now;

    now = __HAL_TIM_GET_COUNTER(&htim2);

    HalfPeriod = now - LastTime;

    LastTime = now;

    LastPulseTick = HAL_GetTick();

    SyncCounter++;

    SyncPresent = 1;

    Device_OnZeroCross();
}

/*----------------------------------------------------------
    Background processing
----------------------------------------------------------*/

void SYNC_Update(void)
{
    if((HAL_GetTick() - LastPulseTick) > 30)
    {
        SyncPresent = 0;
    }
}

/*----------------------------------------------------------
    Getters
----------------------------------------------------------*/

uint8_t SYNC_IsPresent(void)
{
    return SyncPresent;
}

uint32_t SYNC_GetHalfPeriodUs(void)
{
    return HalfPeriod;
}

uint32_t SYNC_GetCounter(void)
{
    return SyncCounter;
}

uint32_t SYNC_GetFrequency_x10(void)
{
    if(HalfPeriod == 0)
    {
        return 0;
    }

    return 5000000UL / HalfPeriod;
}
