#include "settings.h"

#include "tiristor.h"
#include "device.h"

Settings_t Settings;

const uint32_t SETTINGS_Rs485BaudTable[SETTINGS_RS485_BAUD_COUNT] =
{
    9600U,
    19200U,
    38400U,
    57600U,
    115200U
};

/*=============================================================
 * Инициализация значениями по умолчанию
 *=============================================================*/

void SETTINGS_Init(void)
{
    Settings.PulseWidthUs   = 100U;
    Settings.WorkMode       = WORK_MODE_CONTINUOUS;
    Settings.DurationMs     = 500U;

    Settings.ControlMode    = CONTROL_MODE_LOCAL;

    Settings.Rs485BaudIndex = 0U;      /* 9600 */
    Settings.Rs485Address   = 1U;
    Settings.Rs485Parity    = RS485_PARITY_NONE;

    SETTINGS_Apply();
}

/*=============================================================
 * Применение настроек к модулям
 *=============================================================*/

void SETTINGS_Apply(void)
{
    Tiristor_SetPulseWidthUs(Settings.PulseWidthUs);

    /* В режиме FIRST_WAVE импульс формируется только на первой полуволне,
       остальные полуволны тиристор не трогает - этим управляет device.c */
    Tiristor_SetMode((Settings.WorkMode == WORK_MODE_FIRST_WAVE)
                     ? TIRISTOR_MODE_FIRST
                     : TIRISTOR_MODE_ALL);

    Device.Remote = (Settings.ControlMode == CONTROL_MODE_REMOTE) ? 1U : 0U;

    /* RS485 применяется при инициализации драйвера UART (пока не реализован) */
}

uint32_t SETTINGS_GetRs485Baud(void)
{
    if(Settings.Rs485BaudIndex >= SETTINGS_RS485_BAUD_COUNT)
    {
        return SETTINGS_Rs485BaudTable[0];
    }

    return SETTINGS_Rs485BaudTable[Settings.Rs485BaudIndex];
}
