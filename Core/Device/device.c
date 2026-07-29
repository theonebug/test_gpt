#include "device.h"
#include "bsp_lcd.h"
#include "bsp_led.h"
#include "sync.h"
#include "tiristor.h"
#include "settings.h"
#include "tim.h"


Device_t Device;

static volatile uint32_t ZeroCrossCounter = 0;

/* Счётчики текущего запуска */
static volatile uint32_t TestStartTick = 0;
static volatile uint32_t TestHalfWaves = 0;

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

uint8_t Device_IsIdle(void)
{
    return (Device.State == DEVICE_READY) ? 1U : 0U;
}

void Device_Start(void)
{
    if(Device.State != DEVICE_READY)
    {
        return;
    }

    /* Без синхронизации запускаться нельзя */
    if(!SYNC_IsPresent())
    {
        return;
    }

    TestStartTick = HAL_GetTick();
    TestHalfWaves = 0;

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

        /* Отсчёт времени работы идёт от первой полуволны */
        TestStartTick = HAL_GetTick();
        TestHalfWaves = 0;

        Tiristor_Start();
    }

    if(Device.State == DEVICE_TEST)
    {
        /* WORK_MODE_FIRST_WAVE: срезается только первая полуволна,
           остальные идут без искажений до конца заданного времени */
        if((Settings.WorkMode != WORK_MODE_FIRST_WAVE) ||
           (TestHalfWaves == 0U))
        {
            Tiristor_OnZeroCross();
        }

        TestHalfWaves++;
        ZeroCrossCounter++;
    }
}

uint32_t Device_GetZeroCrossCounter(void)
{
    return ZeroCrossCounter;
}

uint32_t Device_GetElapsedMs(void)
{
    if((Device.State != DEVICE_TEST) && (Device.State != DEVICE_FINISHED))
    {
        return 0U;
    }

    return HAL_GetTick() - TestStartTick;
}

/* Аварийный таймаут: в непрерывном режиме остановка возможна только
   кнопкой STOP, поэтому ограничиваем время работы сверху */
static uint8_t Device_MaxRunExpired(void)
{
    if(Device.State != DEVICE_TEST)
    {
        return 0U;
    }

    return (Device_GetElapsedMs() >= (Settings.MaxRunTimeS * 1000U)) ? 1U : 0U;
}

/* Светодиоды отражают состояние устройства, а не нажатия кнопок */
static void Device_UpdateLeds(void)
{
    /* READY только когда прибор действительно готов к пуску */
    BSP_LED_Set(LED_READY,
                ((Device.State == DEVICE_READY) && Device.SyncOK)
                        ? GPIO_PIN_SET : GPIO_PIN_RESET);

    BSP_LED_Set(LED_PULSE,
                Tiristor_IsActive() ? GPIO_PIN_SET : GPIO_PIN_RESET);

    BSP_LED_Set(LED_ALARM,
                Device.SyncOK ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void Device_Update(void)
{
    Device.SyncOK = SYNC_IsPresent();

    /* Потеря синхронизации во время работы - аварийный стоп */
    if(!Device.SyncOK && (Device.State != DEVICE_READY))
    {
        Device_Stop();
    }

    if(Device_MaxRunExpired())
    {
        Device.State = DEVICE_FINISHED;

        Device_Stop();
    }

    Device_UpdateLeds();

    if(BSP_LCD_GetScreen() != LCD_SCREEN_MAIN)
    {
        /* На экране меню главный экран не трогаем, но логику ведём */
        if((Device.State == DEVICE_TEST) &&
           (Settings.WorkMode != WORK_MODE_CONTINUOUS) &&
           (Device_GetElapsedMs() >= Settings.DurationMs))
        {
            Device.State = DEVICE_FINISHED;

            Device_Stop();
        }

        return;
    }

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

            if(Settings.WorkMode != WORK_MODE_CONTINUOUS)
            {
                uint32_t elapsed = Device_GetElapsedMs();

                if(elapsed >= Settings.DurationMs)
                {
                    Device.State = DEVICE_FINISHED;
                }
            }

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
