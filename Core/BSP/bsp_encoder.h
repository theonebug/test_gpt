/******************************************************************************
 * @file    bsp_encoder.h
 * @brief   Rotary encoder driver (TIM4 Encoder Interface)
 *
 * Hardware:
 *      TIM4 Encoder Mode
 *      PB6 - ENC_A
 *      PB7 - ENC_B
 *
 * Author : GPT + Алексей
 * Version: 0.1.0
 ******************************************************************************/

#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#include "main.h"
#include <stdint.h>

/*-----------------------------------------------------------------------------
 * Public functions
 *----------------------------------------------------------------------------*/

void BSP_Encoder_Init(void);

void BSP_Encoder_Update(void);

void BSP_Encoder_Reset(void);

/* Возвращает количество механических шагов
 *
 * >0  - вращение вправо
 * <0  - вращение влево
 *  0  - вращения не было
 */
int16_t BSP_Encoder_GetDelta(void);

/* Настройка количества аппаратных отсчетов,
 * соответствующих одному механическому щелчку.
 *
 * KY-040 обычно = 4
 */
void BSP_Encoder_SetResolution(uint8_t step);

#endif
