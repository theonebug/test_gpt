#include "device.h"
#include "bsp_lcd.h"
#include "sync.h"
#include "tiristor.h"
#include "tim.h"


Device_t Device;

static volatile uint32_t ZeroCrossCounter = 0;

void Device_Init(void)
{
    Device.Angle = 90;

    Device.PulseDuration = 100;

    Device.SyncOK = 0;

    Device.RS485OK = 0;

    Device.Remote = 0;

    Device.State = DEVICE_READY;
}

void Device_SetAngle(uint16_t angle)
{
    if(angle < 5)
    {
        angle = 5;
    }

    if(angle > 175)
    {
        angle = 175;
    }

    if(Device.Angle == angle)
    {
        return;
    }

    Device.Angle = angle;

    BSP_LCD_UpdateAngle(Device.Angle);
}

uint16_t Device_GetAngle(void)
{
    return Device.Angle;
}

void Device_Start(void)
{
    if(Device.State != DEVICE_READY)
    {
        return;
    }

    Tiristor_Start();

    Device.State = DEVICE_WAIT_SYNC;
}

void Device_Stop(void)
{
    Tiristor_Stop();

    Device.State = DEVICE_READY;
}

DeviceState_t Device_GetState(void)
{
    return Device.State;
}

void Device_OnZeroCross(void)
{
    if(Device.State == DEVICE_WAIT_ZERO)
    {
        Device.State = DEVICE_TEST;

        Tiristor_Start();
    }

    if(Device.State == DEVICE_TEST)
    {
        Tiristor_OnZeroCross();
        ZeroCrossCounter++;
    }
}

uint32_t Device_GetZeroCrossCounter(void)
{
    return ZeroCrossCounter;
}

void Device_Update(void)
{
    Device.SyncOK = SYNC_IsPresent();

    switch(Device.State)
    {
        case DEVICE_READY:

            BSP_LCD_UpdateStatus("READY", UI_COLOR_OK);

            break;

        case DEVICE_WAIT_SYNC:

            BSP_LCD_UpdateStatus("WAIT SYNC", UI_COLOR_WARNING);

            if(Device.SyncOK)
            {
                Device.State = DEVICE_WAIT_ZERO;
            }

            break;

        case DEVICE_WAIT_ZERO:

            BSP_LCD_UpdateStatus("WAIT ZERO", UI_COLOR_TITLE);

            break;

        case DEVICE_TEST:

            BSP_LCD_UpdateStatus("TEST", UI_COLOR_OK);

            //BSP_LCD_UpdateDuration((uint16_t)__HAL_TIM_GET_COUNTER(&htim1));
            //BSP_LCD_UpdateDuration((uint16_t)Tiristor_GetDelayIrqCounter());

            break;

        case DEVICE_FINISHED:

            BSP_LCD_UpdateStatus("FINISHED", UI_COLOR_TITLE);

            Device_Stop();

            break;

        default:

            Device.State = DEVICE_READY;

            break;
    }
}
