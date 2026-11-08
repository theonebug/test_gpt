/*
 * hal_mock.c - реализация фиктивных (mock) функций HAL STM32 для хост-тестов.
 * Также реализует наблюдаемые переменные для заглушек BSP_LCD/BSP_LED
 * и обработчики каналов сравнения TIM2.
 */

#include "stm32f1xx_hal.h"
#include "bsp_lcd.h"
#include "bsp_led.h"
#include "ui_colors.h"
#include "main.h"
#include "tiristor.h"

/*=============================================================
 * "Железные" объекты периферии
 *=============================================================*/
GPIO_TypeDef GPIOC_obj;
GPIO_TypeDef GPIOA_obj;
GPIO_TypeDef GPIOB_obj;

TIM_TypeDef TIM2_;
TIM_TypeDef TIM3_;

TIM_HandleTypeDef htim2 = { &TIM2_ };
TIM_HandleTypeDef htim3 = { &TIM3_ };
TIM_HandleTypeDef htim4 = { NULL };

/*=============================================================
 * Mock-источник времени
 *=============================================================*/
static volatile uint32_t MockTick = 0U;

void HOST_Mock_SetTick(uint32_t t)   { MockTick = t; }
void HOST_Mock_AdvanceTick(uint32_t d){ MockTick += d; }
uint32_t HAL_GetTick(void)           { return MockTick; }

/*=============================================================
 * GPIO
 *
 * BSRR/BRR в реальном GPIO не являются регистрами состояния: запись в них
 * немедленно меняет ODR. Здесь это эмулируется отложенно функцией
 * HOST_Mock_ApplyGpio() (вызывается тестом после обработчиков каналов).
 *=============================================================*/
static void gpio_write(GPIO_TypeDef *port, uint32_t pin,
                       GPIO_PinState state)
{
    if(state == GPIO_PIN_SET)
        port->ODR |= pin;        /* pin - маска, как в реальном HAL */
    else
        port->ODR &= ~pin;
}

void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint32_t pin, GPIO_PinState state)
{
    GPIO_TypeDef *p = port;
    if(p == NULL) return;
    gpio_write(p, pin, state);
}

/*=============================================================
 * TIM
 *=============================================================*/
void HAL_TIM_Base_Start(TIM_HandleTypeDef *htim)  { if(htim->Instance) htim->Instance->CR1 |= TIM_CR1_CEN; }
void HAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim){ HAL_TIM_Base_Start(htim); }
void HAL_TIM_OC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel)
{
    (void)channel;
    if(htim->Instance) htim->Instance->CR1 &= ~TIM_CR1_CEN;
}

/*=============================================================
 * Эмуляция срабатывания каналов сравнения TIM2.
 *
 * Вызываются РЕАЛЬНЫЕ обработчики прибора (Core/Device/tiristor.c):
 * они пишут в BSRR/BRR (как на настоящем железе), после чего
 * HOST_Mock_ApplyGpio() переносит эти записи в ODR.
 *=============================================================*/
void HOST_Mock_ApplyGpio(void)
{
    GPIOC_obj.ODR |=  GPIOC_obj.BSRR;
    GPIOC_obj.ODR &= ~GPIOC_obj.BRR;
    GPIOC_obj.BSRR = 0U;
    GPIOC_obj.BRR  = 0U;

    GPIOA_obj.ODR |=  GPIOA_obj.BSRR;
    GPIOA_obj.ODR &= ~GPIOA_obj.BRR;
    GPIOA_obj.BSRR = 0U;
    GPIOA_obj.BRR  = 0U;

    GPIOB_obj.ODR |=  GPIOB_obj.BSRR;
    GPIOB_obj.ODR &= ~GPIOB_obj.BRR;
    GPIOB_obj.BSRR = 0U;
    GPIOB_obj.BRR  = 0U;
}

void HOST_Mock_FireChannel1(void)
{
    Tiristor_Channel1_IRQHandler();
    HOST_Mock_ApplyGpio();
}

void HOST_Mock_FireChannel2(void)
{
    Tiristor_Channel2_IRQHandler();
    HOST_Mock_ApplyGpio();
}

/*=============================================================
 * Заглушки BSP_LCD (наблюдаемые переменные)
 *=============================================================*/
LCD_Screen_t HOST_LCD_Screen = LCD_SCREEN_MAIN;
uint16_t     HOST_LCD_LastAngle = 0U;
const char  *HOST_LCD_LastStatus = "";

void BSP_LCD_Init(void)            { HOST_LCD_Screen = LCD_SCREEN_SPLASH; }
void BSP_LCD_Clear(void)           { (void)0; }
void BSP_LCD_Invalidate(void)      { (void)0; }
void BSP_LCD_SetScreen(LCD_Screen_t s) { HOST_LCD_Screen = s; }
LCD_Screen_t BSP_LCD_GetScreen(void) { return HOST_LCD_Screen; }
void BSP_LCD_Update(void)          { (void)0; }
void BSP_LCD_UpdateMode(uint8_t r)  { (void)r; }
void BSP_LCD_UpdateRS485(uint8_t o) { (void)o; }
void BSP_LCD_UpdateSync(uint8_t o)  { (void)o; }
void BSP_LCD_UpdateAngle(uint16_t a){ HOST_LCD_LastAngle = a; }
void BSP_LCD_UpdateStatus(const char *t, uint16_t c)
{
    (void)c;
    HOST_LCD_LastStatus = t;
}
void BSP_LCD_UpdateDuration(uint16_t ms) { (void)ms; }
void BSP_LCD_UpdateHeartbeat(void)  { (void)0; }
void BSP_LCD_UpdateDebug(uint16_t a, uint16_t b, uint16_t c,
                         uint16_t d, uint16_t e)
{
    (void)a; (void)b; (void)c; (void)d; (void)e;
}

/*=============================================================
 * Заглушки BSP_LED (наблюдаемые переменные)
 *=============================================================*/
GPIO_PinState HOST_LED_STATE[LED_COUNT] = { GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET };

void BSP_LED_Init(void)            { (void)0; }
void BSP_LED_On(BSP_LED_t led)     { HOST_LED_STATE[led] = GPIO_PIN_SET; }
void BSP_LED_Off(BSP_LED_t led)    { HOST_LED_STATE[led] = GPIO_PIN_RESET; }
void BSP_LED_Toggle(BSP_LED_t led)
{
    HOST_LED_STATE[led] = (HOST_LED_STATE[led] == GPIO_PIN_SET)
                          ? GPIO_PIN_RESET : GPIO_PIN_SET;
}
void BSP_LED_Set(BSP_LED_t led, GPIO_PinState s) { HOST_LED_STATE[led] = s; }

/*=============================================================
 * Общие обработчики (Error/IT)
 *=============================================================*/
void Error_Handler(void)    { (void)0; }
void Watchdog_Refresh(void) { (void)0; }