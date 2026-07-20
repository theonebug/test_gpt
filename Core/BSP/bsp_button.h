/******************************************************************************
 * @file    bsp_button.h
 * @brief   Button BSP Driver
 *
 * Hardware:
 *      BUTTON_RUN      PA7
 *      BUTTON_STOP     PB0
 *      BUTTON_ENCODER  PB8
 *
 * Buttons are active LOW.
 *
 * BSP_Button_Update() must be called every 1 ms.
 *
 * Author: GPT + Алексей
 ******************************************************************************/

#ifndef BSP_BUTTON_H
#define BSP_BUTTON_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"
#include <stdint.h>

/*=============================================================================
 * Button identifiers
 *============================================================================*/

typedef enum
{
    BUTTON_RUN = 0,
    BUTTON_STOP,
    BUTTON_ENCODER,

    BUTTON_COUNT

} ButtonId_t;

/*=============================================================================
 * Public functions
 *============================================================================*/

/**
 * @brief Initialize button driver.
 */
void BSP_Button_Init(void);

/**
 * @brief Button service routine.
 *
 * Must be called every 1 ms.
 */
void BSP_Button_Update(void);

/**
 * @brief Get short press event.
 *
 * Event is automatically cleared after reading.
 *
 * @param id Button identifier
 *
 * @return
 *      1 - short press detected
 *      0 - no event
 */
uint8_t BSP_Button_GetClick(ButtonId_t id);

/**
 * @brief Get long press event.
 *
 * Event is automatically cleared after reading.
 *
 * @param id Button identifier
 *
 * @return
 *      1 - long press detected
 *      0 - no event
 */
uint8_t BSP_Button_GetLongPress(ButtonId_t id);

/**
 * @brief Returns current stable button state.
 *
 * @param id Button identifier
 *
 * @return
 *      1 - pressed
 *      0 - released
 */
uint8_t BSP_Button_IsPressed(ButtonId_t id);

#ifdef __cplusplus
}
#endif

#endif /* BSP_BUTTON_H */
