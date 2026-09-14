#ifndef HOST_BSP_LED_H
#define HOST_BSP_LED_H

/*
 * Хост-заглушка BSP_LED для тестов логики. Состояние светодиодов пишется
 * в массив, доступный тесту. Заменяет Core/BSP/bsp_led.h через include path.
 */

#include "main.h"   /* для GPIO_PinState */

typedef enum
{
    LED_READY = 0,
    LED_ALARM,
    LED_PULSE,
    LED_COUNT
} BSP_LED_t;

void BSP_LED_Init(void);
void BSP_LED_On(BSP_LED_t led);
void BSP_LED_Off(BSP_LED_t led);
void BSP_LED_Toggle(BSP_LED_t led);
void BSP_LED_Set(BSP_LED_t led, GPIO_PinState state);

/* Состояние светодиодов: HOST_LED_STATE[LED_READY] и т.д. */
extern GPIO_PinState HOST_LED_STATE[LED_COUNT];

#endif /* HOST_BSP_LED_H */