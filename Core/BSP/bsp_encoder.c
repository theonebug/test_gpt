/******************************************************************************
 * @file    bsp_encoder.c
 * @brief   Rotary encoder driver
 *
 * Uses TIM4 in Encoder Interface mode.
 *
 * Driver returns mechanical encoder steps instead of raw timer counts.
 *
 * Author : GPT + Алексей
 * Version: 0.1.0
 ******************************************************************************/

#include "bsp_encoder.h"
#include "tim.h"

/*-----------------------------------------------------------------------------
 * Private variables
 *----------------------------------------------------------------------------*/

#define ENCODER_DEFAULT_RESOLUTION    2U
#define ENCODER_MAX_DIFF             10

static int16_t LastCounter = 0;

static int16_t Accumulator = 0;

static uint8_t EncoderResolution = ENCODER_DEFAULT_RESOLUTION;

/*-----------------------------------------------------------------------------
 * Public functions
 *----------------------------------------------------------------------------*/

void BSP_Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

    BSP_Encoder_Reset();
}

void BSP_Encoder_Reset(void)
{
    LastCounter = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);

    Accumulator = 0;
}

void BSP_Encoder_SetResolution(uint8_t step)
{
    if(step == 0)
    {
        return;
    }

    EncoderResolution = step;
}

void BSP_Encoder_Update(void)
{
    int16_t current;
    int16_t diff;

    current = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);

    diff = current - LastCounter;

    LastCounter = current;

    if ((diff > ENCODER_MAX_DIFF) || (diff < -ENCODER_MAX_DIFF))
    {
        return;
    }

    Accumulator += diff;


}

int16_t BSP_Encoder_GetDelta(void)
{
    int16_t delta = 0;

    while(Accumulator >= EncoderResolution)
    {
        Accumulator -= EncoderResolution;
        delta++;
    }

    while(Accumulator <= -EncoderResolution)
    {
        Accumulator += EncoderResolution;
        delta--;
    }

    return delta;
}
