/*
 * bsp_led.h
 *
 *  Created on: 13 июл. 2026 г.
 *      Author: torr
 */
#ifndef BSP_LED_H
#define BSP_LED_H

#include "main.h"

typedef enum
{
    LED_READY = 0,
    LED_ALARM,
    LED_PULSE,

    LED_COUNT

}BSP_LED_t;

void BSP_LED_Init(void);

void BSP_LED_On(BSP_LED_t led);

void BSP_LED_Off(BSP_LED_t led);

void BSP_LED_Toggle(BSP_LED_t led);

void BSP_LED_Set(BSP_LED_t led, GPIO_PinState state);

#endif
