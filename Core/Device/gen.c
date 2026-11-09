#include "gen.h"
#include "main.h"
#include "settings.h"
#include "tim.h"
#include "gpio.h"

/*=============================================================
 * Локальные данные
 *=============================================================*/
static uint8_t GenRunning = 0U;
static uint8_t GenInDelay = 0U;       /* 1 - идёт выдержка угла отсечки   */
static uint32_t GenStartTick = 0U;
static uint32_t GenDurationMs = 0U;
static uint16_t GenFreqTarget = 0U;   /* частота генератора после задержки */

/* TIM1 обрабатывает прерывание по обновлению (UIF).
   При частоте F Гц прерывания приходят с 2*F (скважность 50%).
   Каждое прерывание инвертирует TIRISTOR_OUT. */

/*=============================================================
 * Инициализация TIM1
 *=============================================================*/
void GEN_Init(void)
{
    /* TIM1: APB2 = 72 МГц, Prescaler = 71 -> 1 МГц (1 мкс на тик) */
    __HAL_RCC_TIM1_CLK_ENABLE();

    TIM1->PSC = 71U;          /* 1 мкс тик */
    TIM1->ARR = 1000U;        /* будет переопределено в GEN_Start */
    TIM1->CR1 = TIM_CR1_URS;  /* только переполнение сounters generates UIF */
    TIM1->DIER = TIM_DIER_UIE;/* разрешить прерывание по обновлению */
    TIM1->EGR = TIM_EGR_UG;   /* обновить регистры */
    TIM1->SR = 0;             /* сбросить флаги */

    /* NVIC: TIM1 обновление */
    NVIC_SetPriority(TIM1_UP_IRQn, 1);
    NVIC_EnableIRQ(TIM1_UP_IRQn);

    GenRunning = 0U;
    GenStartTick = 0U;
    GenDurationMs = 0U;
}

/*=============================================================
 * Запуск / остановка
 *=============================================================*/
void GEN_Start(uint16_t freqHz)
{
    if(freqHz == 0U)
    {
        return;
    }

    /* Останавливаем TIM2 (импульс тиристора) чтобы не Conflict с генератором на PC14 */
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_CC2IE);
    HAL_GPIO_WritePin(TIRISTOR_OUT_GPIO_Port, TIRISTOR_OUT_Pin, GPIO_PIN_RESET);

    /* Прерывания с частотой 2*freq для скважности 50%.
       Период между переключениями = 1 000 000 / (2*freq) мкс */
    uint32_t periodUs = 1000000UL / (2UL * (uint32_t)freqHz);

    if(periodUs < 100U) { periodUs = 100U; }   /* ограничение: max 5 кГц */
    if(periodUs > 10000U) { periodUs = 10000U; }/* ограничение: min 50 Гц */

    TIM1->ARR = periodUs - 1U;
    TIM1->CNT = 0;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR = ~TIM_SR_UIF;
    TIM1->DIER |= TIM_DIER_UIE;   /* разрешить прерывания */

    TIM1->CR1 |= TIM_CR1_CEN;      /* запуск счётчика */

    GenRunning = 1U;
    GenStartTick = HAL_GetTick();
    GenDurationMs = Settings.DurationMs;
}

void GEN_Stop(void)
{
    TIM1->CR1 &= ~TIM_CR1_CEN;     /* остановка счётчика */
    TIM1->DIER &= ~TIM_DIER_UIE;   /* запретить прерывания */
    TIM1->SR = ~TIM_SR_UIF;

    HAL_GPIO_WritePin(TIRISTOR_OUT_GPIO_Port, TIRISTOR_OUT_Pin, GPIO_PIN_RESET);

    GenRunning = 0U;
    GenInDelay = 0U;
    GenFreqTarget = 0U;
    GenStartTick = 0U;
    GenDurationMs = 0U;
}

/*=============================================================
 * Отложенный запуск: выдержка delayUs мкс, затем генератор freqHz
 *=============================================================*/
void GEN_StartDelayed(uint16_t delayUs, uint16_t freqHz)
{
    if(freqHz == 0U)
    {
        return;
    }

    /* Останавливаем TIM2 чтобы не было конфликта на PC14 */
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    TIM2->CR1 &= ~TIM_CR1_CEN;
    TIM2->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_CC2IE);
    HAL_GPIO_WritePin(TIRISTOR_OUT_GPIO_Port, TIRISTOR_OUT_Pin, GPIO_PIN_RESET);

    /* Запоминаем целевую частоту генератора */
    GenFreqTarget = freqHz;

    /* Настраиваем TIM1 на выдержку delayUs мкс (одно прерывание) */
    if(delayUs < 10U) { delayUs = 10U; }      /* минимум 10 мкс */
    if(delayUs > 65000U) { delayUs = 65000U; }/* максимум для 16 бит при 1 мкс тике */

    TIM1->ARR = delayUs - 1U;
    TIM1->CNT = 0;
    TIM1->EGR = TIM_EGR_UG;
    TIM1->SR = ~TIM_SR_UIF;
    TIM1->DIER |= TIM_DIER_UIE;

    GenInDelay = 1U;
    GenRunning = 1U;
    GenStartTick = HAL_GetTick();
    GenDurationMs = Settings.DurationMs;

    TIM1->CR1 |= TIM_CR1_CEN;      /* запуск счётчика на выдержку */
}

/*=============================================================
 * Статус
 *=============================================================*/
uint8_t GEN_IsRunning(void)
{
    return GenRunning;
}

/*=============================================================
 * Фоновая обработка: остановка по истечении времени
 *=============================================================*/
void GEN_Process(void)
{
    if(!GenRunning)
    {
        return;
    }

    if(GenDurationMs > 0U)
    {
        uint32_t elapsed = HAL_GetTick() - GenStartTick;
        if(elapsed >= GenDurationMs)
        {
            GEN_Stop();
        }
    }
}

/*=============================================================
 * Прерывание TIM1 (инвертирование выхода)
 *=============================================================*/
void GEN_IRQHandler(void)
{
    if((TIM1->SR & TIM_SR_UIF) != 0U)
    {
        TIM1->SR = ~TIM_SR_UIF;   /* сброс флага UIF */

        if(GenInDelay)
        {
            /* Выдержка угла отсечки окончена — переходим к генерации */
            GenInDelay = 0U;

            if(GenFreqTarget != 0U)
            {
                /* Прерывания с частотой 2*freq для скважности 50%.
                   Период между переключениями = 1 000 000 / (2*freq) мкс */
                uint32_t periodUs = 1000000UL / (2UL * (uint32_t)GenFreqTarget);

                if(periodUs < 100U) { periodUs = 100U; }   /* ограничение: max 5 кГц */
                if(periodUs > 10000U) { periodUs = 10000U; }/* ограничение: min 50 Гц */

                TIM1->ARR = periodUs - 1U;
                TIM1->CNT = 0;
                TIM1->EGR = TIM_EGR_UG;
                TIM1->SR = ~TIM_SR_UIF;

                /* Начальное состояние: выход в 0 */
                HAL_GPIO_WritePin(TIRISTOR_OUT_GPIO_Port, TIRISTOR_OUT_Pin, GPIO_PIN_RESET);
            }
        }
        else
        {
            /* Инвертируем выход тиристора ( генератор уже работает) */
            GPIO_TypeDef *port = TIRISTOR_OUT_GPIO_Port;
            uint16_t pin = TIRISTOR_OUT_Pin;

            if((port->ODR & pin) != 0U)
            {
                port->BRR = pin;   /* сброс */
            }
            else
            {
                port->BSRR = pin;  /* установка */
            }
        }
    }
}
