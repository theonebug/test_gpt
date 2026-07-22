#include "device.h"
#include "bsp_lcd.h"

Device_t Device;

void Device_Init(void)
{
    Device.Angle = 90;

    Device.PulseDuration = 100;

    Device.SyncOK = 0;

    Device.RS485OK = 0;

    Device.Remote = 0;
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


