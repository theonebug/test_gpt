#include "tiristor.h"
#include "device.h"
#include "sync.h"
#include "tim.h"
#include "gpio.h"

/*=============================================================
 * Локальные данные
 *=============================================================*/

typedef struct
{
    uint8_t Active;
    TiristorMode_t Mode;

    uint16_t DelayUs;
    uint16_t PulseWidthUs;

} TiristorControl_t;

static TiristorControl_t Tiristor;

#define TIRISTOR_MIN_DELAY_US   10U
#define TIRISTOR_GUARD_US       50U

static volatile uint32_t CH1Counter = 0;
static volatile uint32_t CH2Counter = 0;

/* Заход в OnZeroCross, когда предыдущий импульс ещё не завершён */
static volatile uint32_t RestartCounter = 0;

/*=============================================================
 * Инициализация
 *=============================================================*/

void Tiristor_Init(void)
{
    Tiristor.Active = 0;

    Tiristor.Mode = TIRISTOR_MODE_FIRST;

    Tiristor.DelayUs = 0;

    Tiristor.PulseWidthUs = 100;

    HAL_GPIO_WritePin(TIRISTOR_OUT_GPIO_Port,
                      TIRISTOR_OUT_Pin,
                      GPIO_PIN_RESET);

    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);

    __HAL_TIM_SET_COUNTER(&htim2, 0);
}

/*=============================================================
 * Управление
 *=============================================================*/

void Tiristor_Start(void)
{
    Tiristor.Active = 1;
}

void Tiristor_Stop(void)
{
    Tiristor.Active = 0;

    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);

    HAL_GPIO_WritePin(TIRISTOR_OUT_GPIO_Port,
                      TIRISTOR_OUT_Pin,
                      GPIO_PIN_RESET);
}

uint8_t Tiristor_IsActive(void)
{
    return Tiristor.Active;
}

/*=============================================================
 * Параметры
 *=============================================================*/

void Tiristor_SetMode(TiristorMode_t mode)
{
    Tiristor.Mode = mode;
}

TiristorMode_t Tiristor_GetMode(void)
{
    return Tiristor.Mode;
}

void Tiristor_SetDelayUs(uint16_t delay)
{
    Tiristor.DelayUs = delay;
}

uint16_t Tiristor_GetDelayUs(void)
{
    return Tiristor.DelayUs;
}

void Tiristor_SetPulseWidthUs(uint16_t width)
{
    Tiristor.PulseWidthUs = width;
}

uint16_t Tiristor_GetPulseWidthUs(void)
{
    return Tiristor.PulseWidthUs;
}

uint32_t Tiristor_GetCH1Counter(void)
{
    return CH1Counter;
}

uint32_t Tiristor_GetCH2Counter(void)
{
    return CH2Counter;
}

uint32_t Tiristor_GetRestartCounter(void)
{
    return RestartCounter;
}

/*=============================================================
 * Переход через ноль
 *=============================================================*/

void Tiristor_OnZeroCross(void)
{
    if(Tiristor.Active == 0) { return; }

    uint32_t halfPeriod = SYNC_GetHalfPeriodUs();
    uint32_t pulseWidth = Tiristor.PulseWidthUs;

    // Без достоверного полупериода угол посчитать нельзя - пропускаем полупериод
    if(halfPeriod <= (pulseWidth + TIRISTOR_GUARD_US)) { return; }

    uint32_t max_delay = halfPeriod - pulseWidth - TIRISTOR_GUARD_US;
    uint32_t delay = (halfPeriod * Device_GetAngle()) / 180U;

    if(delay < TIRISTOR_MIN_DELAY_US) { delay = TIRISTOR_MIN_DELAY_US; }
    if(delay > max_delay)             { delay = max_delay; }

    Tiristor.DelayUs = (uint16_t)delay;

    if(TIM2->CR1 & TIM_CR1_CEN)
    {
        /* Предыдущий полупериод не успел закончиться */
        RestartCounter++;
    }

    // Порядок важен: стоп -> CCR -> UG -> сброс флагов -> разрешение IRQ -> старт.
    // Запуск строго последним, иначе при малых углах сброс SR затирает уже
    // взведённый CC1IF и импульс теряется.
    TIM2->CR1  &= ~TIM_CR1_CEN;
    TIM2->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_CC2IE);
    TIM2->CNT   = 0;
    TIM2->CCR1  = delay;
    TIM2->CCR2  = delay + pulseWidth;
    TIM2->EGR   = TIM_EGR_UG;
    TIM2->SR    = 0;
    TIM2->DIER |= (TIM_DIER_CC1IE | TIM_DIER_CC2IE);
    TIM2->CR1  |= TIM_CR1_CEN;
}

/*=============================================================
 * Channel 1 Compare
 *=============================================================*/

void Tiristor_Channel1_IRQHandler(void)
{
    TIRISTOR_OUT_GPIO_Port->BSRR = TIRISTOR_OUT_Pin;
    CH1Counter++;
    if(Tiristor.Mode == TIRISTOR_MODE_FIRST)
    {
        htim2.Instance->DIER &= ~TIM_DIER_CC1IE;
    }
}

/*=============================================================
 * Channel 2 Compare
 *=============================================================*/

void Tiristor_Channel2_IRQHandler(void)
{

	TIRISTOR_OUT_GPIO_Port->BRR = TIRISTOR_OUT_Pin;
	TIM2->CR1 &= ~TIM_CR1_CEN;
	CH2Counter++;
}
