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

/* Слепое окно после перехода через ноль (защита от дребезга детектора) */
#define SYNC_BLANKING_US          5000U

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
    static uint16_t lastCapture = 0;

    uint16_t now;
    uint16_t diff;

    /* TIM3 тикает по 1 мкс и переполняется каждые 65536 мкс. Полупериод
       (10 мс) сильно меньше этого, поэтому разность в 16 битах всегда верна
       сама по себе - счётчик переполнений и гонка с TIM3_IRQn больше не нужны. */
    now  = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
    diff = (uint16_t)(now - lastCapture);

    lastCapture = now;

    /* Слишком близкое срабатывание - дребезг детектора, игнорируем событие */
    if(diff < SYNC_BLANKING_US)
    {
        GlitchCounter++;
        return;
    }

    /* Период обновляем только по правдоподобному измерению (40...70 Гц),
       но само событие отрабатываем в любом случае - импульс терять нельзя */
    if((diff >= SYNC_HALF_PERIOD_MIN_US) && (diff <= SYNC_HALF_PERIOD_MAX_US))
    {
        HalfPeriod = diff;
    }
    else
    {
        GlitchCounter++;
    }

    if(HalfPeriod == 0U)
    {
        /* Ещё ни одного достоверного измерения */
        return;
    }

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
