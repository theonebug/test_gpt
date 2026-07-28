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
static volatile uint32_t GlitchCounter = 0;

#define SYNC_HALF_PERIOD_MIN_US   7000U    // ~70 Гц
#define SYNC_HALF_PERIOD_MAX_US   12500U   // ~40 Гц

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
    GlitchCounter = 0;
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
    uint32_t now;
    uint32_t currentOverflows;
    uint64_t currentTime;
    static uint64_t lastTime = 0; // Делаем переменную статической для сохранения между вызовами
    uint64_t diff;

    // Атомарное чтение текущего счётчика и переполнений.
    // EXTI имеет более высокий приоритет, чем TIM3_IRQn, поэтому мы можем
    // попасть сюда после переполнения TIM3, но до того как отработал его
    // обработчик и инкрементировал LastOverflows. Учитываем это по флагу UIF.
    __disable_irq();
    now = __HAL_TIM_GET_COUNTER(&htim3);
    currentOverflows = LastOverflows;
    if ((htim3.Instance->SR & TIM_SR_UIF) && (now < 0x8000U))
    {
        currentOverflows++;
    }
    __enable_irq();

    // Вычисляем текущее 64-битное время
    currentTime = ((uint64_t)currentOverflows << 16) | now;

    // Считаем разницу (при первом запуске diff может быть некорректным, это нормально)
    diff = currentTime - lastTime;

    // Сохраняем текущее время для следующего прерывания
    lastTime = currentTime;

    // Принимаем только правдоподобный полупериод (40...70 Гц), остальное - помеха
    if ((diff >= SYNC_HALF_PERIOD_MIN_US) && (diff <= SYNC_HALF_PERIOD_MAX_US))
    {
        HalfPeriod = (uint32_t)diff;

        // Обновляем остальные флаги
        LastPulseTick = HAL_GetTick();
        SyncCounter++;
        SyncPresent = 1;

        Device_OnZeroCross();
    }
    else
    {
        GlitchCounter++;
    }
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

uint32_t SYNC_GetGlitchCounter(void)
{
    return GlitchCounter;
}

uint32_t SYNC_GetFrequency_x10(void)
{
    if (HalfPeriod == 0)
    {
        return 0;
    }

    return 5000000UL / HalfPeriod;
}
