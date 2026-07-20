/******************************************************************************
 * @file    input.c
 * @brief   Input Manager
 ******************************************************************************/

#include "input.h"

#include "bsp_button.h"
#include "bsp_encoder.h"
#include "event.h"

/*=============================================================================
 * Initialization
 *============================================================================*/

void INPUT_Init(void)
{
    BSP_Button_Init();
    BSP_Encoder_Init();
}

/*=============================================================================
 * Update
 *============================================================================*/

void INPUT_Update(void)
{
    int16_t delta;

    //BSP_Button_Update();

    BSP_Encoder_Update();

    /*----------------------------------------------------------
      RUN
    ----------------------------------------------------------*/

    if(BSP_Button_GetClick(BUTTON_RUN))
    {
        EVENT_Push(EVENT_RUN_CLICK,0);
    }

    if(BSP_Button_GetLongPress(BUTTON_RUN))
    {
        EVENT_Push(EVENT_RUN_LONG,0);
    }

    /*----------------------------------------------------------
      STOP
    ----------------------------------------------------------*/

    if(BSP_Button_GetClick(BUTTON_STOP))
    {
        EVENT_Push(EVENT_STOP_CLICK,0);
    }

    if(BSP_Button_GetLongPress(BUTTON_STOP))
    {
        EVENT_Push(EVENT_STOP_LONG,0);
    }

    /*----------------------------------------------------------
      Encoder button
    ----------------------------------------------------------*/

    if(BSP_Button_GetClick(BUTTON_ENCODER))
    {
        EVENT_Push(EVENT_ENCODER_CLICK,0);
    }

    if(BSP_Button_GetLongPress(BUTTON_ENCODER))
    {
        EVENT_Push(EVENT_ENCODER_LONG,0);
    }

    /*----------------------------------------------------------
      Encoder rotation
    ----------------------------------------------------------*/

    delta = BSP_Encoder_GetDelta();

    while(delta > 0)
    {
        EVENT_Push(EVENT_ENCODER_RIGHT,1);
        delta--;
    }

    while(delta < 0)
    {
        EVENT_Push(EVENT_ENCODER_LEFT,-1);
        delta++;
    }
}
