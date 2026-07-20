/******************************************************************************
 * @file    bsp_button.c
 * @brief   Button BSP
 *
 * Update period:
 *      BSP_Button_Update() must be called every 1 ms.
 *
 ******************************************************************************/

#include "bsp_button.h"

/*=============================================================================
 * Configuration
 *============================================================================*/

#define BUTTON_DEBOUNCE_MS      20U
#define BUTTON_LONG_PRESS_MS  1000U

/*=============================================================================
 * Types
 *============================================================================*/

typedef struct
{
    GPIO_TypeDef *Port;
    uint16_t Pin;

    uint8_t RawState;          // последнее считанное состояние GPIO
    uint8_t State;             // подтвержденное состояние

    uint8_t DebounceCnt;

    uint16_t PressTime;

    uint8_t LongDetected;

} Button_t;

/*=============================================================================
 * Private variables
 *============================================================================*/

static Button_t Buttons[BUTTON_COUNT] =
{
    {GPIOA, GPIO_PIN_7, 0,0,0,0,0},      // RUN

    {GPIOB, GPIO_PIN_0, 0,0,0,0,0},      // STOP

    {GPIOB, GPIO_PIN_8, 0,0,0,0,0}       // ENCODER
};

static uint8_t ClickEvent[BUTTON_COUNT];

static uint8_t LongEvent[BUTTON_COUNT];

/*=============================================================================
 * Init
 *============================================================================*/

void BSP_Button_Init(void)
{
    for(uint8_t i=0;i<BUTTON_COUNT;i++)
    {
        Buttons[i].RawState=0;
        Buttons[i].State=0;
        Buttons[i].DebounceCnt=0;
        Buttons[i].PressTime=0;
        Buttons[i].LongDetected=0;

        ClickEvent[i]=0;
        LongEvent[i]=0;
    }
}

/*=============================================================================
 * Update (1ms)
 *============================================================================*/

void BSP_Button_Update(void)
{
    for(uint8_t i=0;i<BUTTON_COUNT;i++)
    {
        Button_t *b=&Buttons[i];

        uint8_t raw=
            (HAL_GPIO_ReadPin(b->Port,b->Pin)==GPIO_PIN_RESET);

        /*------------------------------------------------------
          Обнаружено изменение входа
        ------------------------------------------------------*/

        if(raw!=b->RawState)
        {
            b->RawState=raw;

            b->DebounceCnt=BUTTON_DEBOUNCE_MS;
        }

        /*------------------------------------------------------
          Антидребезг
        ------------------------------------------------------*/

        if(b->DebounceCnt)
        {
            b->DebounceCnt--;

            if(b->DebounceCnt==0)
            {
                if(b->State!=b->RawState)
                {
                    b->State=b->RawState;

                    /*-------------------------
                      Нажали
                    -------------------------*/

                    if(b->State)
                    {
                        b->PressTime=0;

                        b->LongDetected=0;
                    }

                    /*-------------------------
                      Отпустили
                    -------------------------*/

                    else
                    {
                        if(!b->LongDetected)
                        {
                            ClickEvent[i]=1;
                        }
                    }
                }
            }
        }

        /*------------------------------------------------------
          Долгое нажатие
        ------------------------------------------------------*/

        if(b->State)
        {
            if(b->PressTime < BUTTON_LONG_PRESS_MS)
            {
                b->PressTime++;

                if(b->PressTime==BUTTON_LONG_PRESS_MS)
                {
                    b->LongDetected=1;

                    LongEvent[i]=1;
                }
            }
        }
    }
}

/*=============================================================================
 * Events
 *============================================================================*/

uint8_t BSP_Button_GetClick(ButtonId_t id)
{
    uint8_t event;

    if(id>=BUTTON_COUNT)
        return 0;

    event=ClickEvent[id];

    ClickEvent[id]=0;

    return event;
}

uint8_t BSP_Button_GetLongPress(ButtonId_t id)
{
    uint8_t event;

    if(id>=BUTTON_COUNT)
        return 0;

    event=LongEvent[id];

    LongEvent[id]=0;

    return event;
}

/*=============================================================================
 * State
 *============================================================================*/

uint8_t BSP_Button_IsPressed(ButtonId_t id)
{
    if(id>=BUTTON_COUNT)
        return 0;

    return Buttons[id].State;
}
