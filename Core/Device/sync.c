#include "sync.h"
#include "main.h"
#include "device.h"
#include "tim.h"

/*----------------------------------------------------------
    Variables
----------------------------------------------------------*/

static volatile uint32_t SyncCounter = 0;
static volatile uint16_t LastTime = 0;           // 16-бит
static volatile uint32_t LastOverflows = 0;      // счётчик переполнений
static volatile uint32_t HalfPeriod = 0;
static volatile uint8_t SyncPresent = 0;
static volatile uint32_t LastPulseTick = 0;
static volatile uint32_t TestData = 0;

/*----------------------------------------------------------
    Initialization
----------------------------------------------------------*/

void SYNC_Init(void)
{
	// Запускаем TIM3
    HAL_TIM_Base_Start(&htim3);
    HAL_TIM_Base_Start_IT(&htim3);  // Включаем прерывание по переполнению
    //TestData = htim3.Instance->CR1;


    // Сбрасываем счётчик
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    LastOverflows = 0;
    LastTime = 0;

    SyncCounter = 0;
    HalfPeriod = 0;
    SyncPresent = 0;
    LastPulseTick = HAL_GetTick();
}

/*----------------------------------------------------------
    TIM3 переполнение
----------------------------------------------------------*/

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3)
    {
        LastOverflows++;  // увеличиваем счётчик переполнений
    }
}

/*----------------------------------------------------------
    EXTI callback
----------------------------------------------------------*/

void SYNC_EXTI_Handler(void)
{

	Device_OnZeroCross();
    uint32_t now;
    uint32_t currentOverflows;
    uint64_t currentTime;
    static uint64_t lastTime = 0; // Делаем переменную статической для сохранения между вызовами
    uint64_t diff;

    // Атомарное чтение текущего счётчика и переполнений
    __disable_irq();
    now = __HAL_TIM_GET_COUNTER(&htim3);
    currentOverflows = LastOverflows;
    __enable_irq();

    // Вычисляем текущее 64-битное время
    currentTime = ((uint64_t)currentOverflows << 16) | now;

    // Считаем разницу (при первом запуске diff может быть некорректным, это нормально)
    diff = currentTime - lastTime;
    HalfPeriod = (uint32_t)diff;

    // Сохраняем текущее время для следующего прерывания
    lastTime = currentTime;

    // Обновляем остальные флаги
    LastPulseTick = HAL_GetTick();
    SyncCounter++;
    SyncPresent = 1;



}

/*----------------------------------------------------------
    Background processing
----------------------------------------------------------*/

void SYNC_Update(void)
{
    if ((HAL_GetTick() - LastPulseTick) > 30)
    {
        SyncPresent = 0;
    }
}

/*----------------------------------------------------------
    Getters
----------------------------------------------------------*/

uint32_t SYNC_TestData(void)
{

           return TestData;
    }


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
    if (HalfPeriod == 0)
    {
        return 0;
    }

    return 5000000UL / HalfPeriod;
}
