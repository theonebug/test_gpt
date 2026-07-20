/*
 * bsp_led.c
 *
 *  Created on: 13 июл. 2026 г.
 *      Author: torr
 */
#include "bsp_led.h"

typedef struct
{
    GPIO_TypeDef *Port;
    uint16_t Pin;

}LED_t;

static const LED_t leds[LED_COUNT]=
{
    {GPIOB,GPIO_PIN_1},      // READY
    {GPIOB,GPIO_PIN_10},     // ALARM
    {GPIOB,GPIO_PIN_11}      // PULSE
};

void BSP_LED_Init(void)
{
    for(uint8_t i=0;i<LED_COUNT;i++)
        BSP_LED_Off((BSP_LED_t)i);
}

void BSP_LED_On(BSP_LED_t led)
{
    HAL_GPIO_WritePin(leds[led].Port,
                      leds[led].Pin,
                      GPIO_PIN_SET);
}

void BSP_LED_Off(BSP_LED_t led)
{
    HAL_GPIO_WritePin(leds[led].Port,
                      leds[led].Pin,
                      GPIO_PIN_RESET);
}

void BSP_LED_Toggle(BSP_LED_t led)
{
    HAL_GPIO_TogglePin(leds[led].Port,
                       leds[led].Pin);
}

void BSP_LED_Set(BSP_LED_t led, GPIO_PinState state)
{
    HAL_GPIO_WritePin(leds[led].Port,
                      leds[led].Pin,
                      state);
}

