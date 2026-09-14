#ifndef STM32F1XX_HAL_H
#define STM32F1XX_HAL_H

/*
 * Упрощённый фиктивный (mock) заголовок HAL STM32F1xx для хост-сборки
 * тестов логики на Windows/MinGW. Вместо настоящих регистров MCU здесь
 * обычные переменные, а вызовы HAL реализованы в hal_mock.c.
 *
 * Служит ЗАМЕНОЙ Drivers/STM32F1xx_HAL_Driver/Inc/stm32f1xx_hal.h,
 * поэтому должен быть найден компилятором раньше реального (include path
 * на host/sim стоит первым).
 */

#include <stdint.h>

/*=============================================================
 * GPIO
 *=============================================================*/
typedef enum
{
    GPIO_PIN_RESET = 0,
    GPIO_PIN_SET
} GPIO_PinState;

/* Номера пинов (0..15) - согласовано с main.h проекта */
enum { GPIO_PIN_0 = 0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4,
       GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_9,
       GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14,
       GPIO_PIN_15 };

/* "Железный" объект GPIO-порта; используются поля, к которым реальный код
   обращается напрямую через стрелку: ODR (выходные), BSRR/BRR (вкл/выкл). */
typedef struct
{
    uint32_t ODR;   /* выходные данные (состояние выводов) */
    uint32_t IDR;   /* входные данные */
    uint32_t BSRR;  /* бит для установки/сброса (BSRR высокие биты) */
    uint32_t BRR;   /* бит сброса */
} GPIO_TypeDef;

/* Псевдопорты прибора (см. main.h проекта) */
extern GPIO_TypeDef GPIOC_obj;
extern GPIO_TypeDef GPIOA_obj;
extern GPIO_TypeDef GPIOB_obj;

/* В реальном HAL GPIOC - это макрос-указатель на регистры.
   Делаем так же, чтобы код с ->ODR / HAL_GPIO_WritePin(GPIOC,...) собирался. */
#define GPIOC (&GPIOC_obj)
#define GPIOA (&GPIOA_obj)
#define GPIOB (&GPIOB_obj)

/*=============================================================
 * TIM (таймеры) - минимум, используемый тиристорной логикой
 *=============================================================*/
typedef struct
{
    uint32_t CNT;   /* счётчик */
    uint32_t CR1;   /* управляющий регистр 1 (вкл/выкл, CEN) */
    uint32_t CCR1;  /* сравнение 1 */
    uint32_t CCR2;  /* сравнение 2 */
    uint32_t SR;    /* флаги/статус */
    uint32_t DIER;  /* разрешения прерываний */
    uint32_t EGR;   /* event generation */
} TIM_TypeDef;

typedef struct
{
    TIM_TypeDef *Instance;
} TIM_HandleTypeDef;

/* Регистры таймеров, к которым код обращается напрямую */
extern TIM_TypeDef TIM2_;
extern TIM_TypeDef TIM3_;

/* Обработчики (переменные) - согласовано с Core/Inc/tim.h */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

/* Макросы регистров: хост-замены имён-регистров на объекты */
#define TIM2 (&TIM2_)
#define TIM3 (&TIM3_)

/* Флаги/биты */
#define TIM_CR1_CEN        ((uint32_t)0x0001)
#define TIM_DIER_CC1IE     ((uint32_t)0x0002)
#define TIM_DIER_CC2IE     ((uint32_t)0x0004)
#define TIM_SR_UIF         ((uint32_t)0x0100)
#define TIM_EGR_UG         ((uint32_t)0x0001)

/* Каналы таймера для HAL_TIM_OC_Stop_IT */
#define TIM_CHANNEL_1      ((uint32_t)1)
#define TIM_CHANNEL_2      ((uint32_t)2)

/*=============================================================
 * Обобщённые имена EXTI (используются в main.h как константы)
 *=============================================================*/
enum { EXTI0_IRQn = 0 };

/*=============================================================
 * HAL-функции (реализация в hal_mock.c)
 *=============================================================*/
uint32_t HAL_GetTick(void);

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint32_t pin, GPIO_PinState state);

void HAL_TIM_Base_Start(TIM_HandleTypeDef *htim);
void HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim);
void HAL_TIM_OC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel);

#define __HAL_TIM_SET_COUNTER(htim, val)   ((htim)->Instance->CNT = (val))
#define __HAL_TIM_GET_COUNTER(htim)        ((htim)->Instance->CNT)

/*=============================================================
 * Обработчики сравнения TIM2, вызываемые симулятором тактов
 *=============================================================*/
void TIM2_CH1_handler(void);
void TIM2_CH2_handler(void);

#endif /* STM32F1XX_HAL_H */