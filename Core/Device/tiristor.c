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

static volatile uint32_t CH1Counter = 0;
static volatile uint32_t CH2Counter = 0;

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

/*=============================================================
 * Переход через ноль
 *=============================================================*/

void Tiristor_OnZeroCross(void)
{
if(Tiristor.Active == 0) { return; } // Счетчик ZeroCross будет инкрементироваться честно, без условий
//ZeroCrossCounter++;

uint32_t halfPeriod = SYNC_GetHalfPeriodUs();

Tiristor.DelayUs = (halfPeriod * Device_GetAngle()) / 180U;
if(Tiristor.DelayUs < 10U) Tiristor.DelayUs = 10U;
uint32_t max_delay = halfPeriod - Tiristor.PulseWidthUs - 50U;
if(Tiristor.DelayUs > max_delay) Tiristor.DelayUs = max_delay;


TIM2->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_CC2IE);
htim2.Instance->EGR = TIM_EGR_UG;
TIM2->CR1 &= ~TIM_CR1_CEN; // остановили таймер
TIM2->SR = 0;
TIM2->CNT = 0;
TIM2->CCR1 = Tiristor.DelayUs;
TIM2->CCR2 = Tiristor.DelayUs + Tiristor.PulseWidthUs;
TIM2->SR = 0;

TIM2->CR1 |= TIM_CR1_CEN;// снова запустили
// 3. Очищаем флаги прерываний (так как EGR->UG принудительно взводит флаг UIF)
//
htim2.Instance->SR = ~(TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_UPDATE);
// 4. Разрешаем прерывания каналов строго на этот полупериод
//
htim2.Instance->DIER |= (TIM_DIER_CC1IE | TIM_DIER_CC2IE);


// 1. Сначала записываем значения в теневые регистры CCR (благодаря Preload это безопасно)
// htim2.Instance->CCR1 = Tiristor.DelayUs;
// htim2.Instance->CCR2 = Tiristor.DelayUs + Tiristor.PulseWidthUs;
//CCRProgramCounter++;
// 2. Генерируем аппаратное событие обновления (Update Event)
// Это мгновенно сбрасывает CNT в 0 и переносит новые CCR в активную работу
//

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
