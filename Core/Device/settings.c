#include "settings.h"

#include <string.h>

#include "main.h"
#include "tiristor.h"
#include "device.h"

/*=============================================================
 * Хранение во Flash
 *
 * Последняя страница STM32F103C8 (64 КБ, страница 1 КБ).
 * Область исключена из кода в STM32F103C8TX_FLASH.ld (FLASH = 63K).
 *=============================================================*/
#define SETTINGS_FLASH_ADDR   0x0800FC00UL
#define SETTINGS_MAGIC        0x53455431UL   /* SET1 */

typedef struct
{
    uint32_t   Magic;
    Settings_t Data;
    uint32_t   Crc;

} SettingsStorage_t;

Settings_t Settings;

/* Запись во Flash останавливает ядро на время стирания страницы,
   поэтому во время работы тиристора сохранение откладывается. */
static uint8_t SaveRequested = 0U;

const uint32_t SETTINGS_Rs485BaudTable[SETTINGS_RS485_BAUD_COUNT] =
{
    9600U,
    19200U,
    38400U,
    57600U,
    115200U
};

/*=============================================================
 * Контрольная сумма и проверка границ
 *=============================================================*/

static uint32_t SETTINGS_Crc(const Settings_t *data)
{
    const uint8_t *p = (const uint8_t *)data;
    uint32_t crc = 0xA5A5A5A5UL;
    uint32_t i;

    for(i = 0U; i < sizeof(Settings_t); i++)
    {
        crc = (crc << 5) ^ (crc >> 27) ^ p[i];
    }

    return crc;
}

static uint8_t SETTINGS_InRange(uint16_t value, uint16_t min, uint16_t max)
{
    return ((value >= min) && (value <= max)) ? 1U : 0U;
}

static uint8_t SETTINGS_CheckRanges(const Settings_t *data)
{
    if(!SETTINGS_InRange(data->PulseWidthUs,
                         SETTINGS_PULSE_WIDTH_MIN_US,
                         SETTINGS_PULSE_WIDTH_MAX_US))     { return 0U; }

    if(!SETTINGS_InRange(data->DurationMs,
                         SETTINGS_DURATION_MIN_MS,
                         SETTINGS_DURATION_MAX_MS))        { return 0U; }

    if(!SETTINGS_InRange(data->Rs485Address,
                         SETTINGS_RS485_ADDR_MIN,
                         SETTINGS_RS485_ADDR_MAX))         { return 0U; }

    if(data->WorkMode       >= WORK_MODE_COUNT)            { return 0U; }
    if(data->ControlMode    >= CONTROL_MODE_COUNT)         { return 0U; }
    if(data->Rs485Parity    >= RS485_PARITY_COUNT)         { return 0U; }
    if(data->Rs485BaudIndex >= SETTINGS_RS485_BAUD_COUNT)  { return 0U; }

    return 1U;
}

/*=============================================================
 * Загрузка из Flash
 *=============================================================*/

static uint8_t SETTINGS_Load(void)
{
    const SettingsStorage_t *storage =
            (const SettingsStorage_t *)SETTINGS_FLASH_ADDR;

    if(storage->Magic != SETTINGS_MAGIC)                      { return 0U; }
    if(storage->Crc   != SETTINGS_Crc(&storage->Data))        { return 0U; }
    if(!SETTINGS_CheckRanges(&storage->Data))                 { return 0U; }

    Settings = storage->Data;

    return 1U;
}

/*=============================================================
 * Сохранение во Flash
 *=============================================================*/

uint8_t SETTINGS_Save(void)
{
    const SettingsStorage_t *storage =
            (const SettingsStorage_t *)SETTINGS_FLASH_ADDR;

    SettingsStorage_t image;
    FLASH_EraseInitTypeDef erase;
    uint32_t pageError = 0U;
    uint32_t address;
    const uint16_t *halfword;
    uint32_t i;

    image.Magic = SETTINGS_MAGIC;
    image.Data  = Settings;
    image.Crc   = SETTINGS_Crc(&image.Data);

    /* Ничего не изменилось - не трогаем Flash */
    if(memcmp(storage, &image, sizeof(image)) == 0)
    {
        return 1U;
    }

    HAL_FLASH_Unlock();

    erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = SETTINGS_FLASH_ADDR;
    erase.NbPages     = 1U;

    if(HAL_FLASHEx_Erase(&erase, &pageError) != HAL_OK)
    {
        HAL_FLASH_Lock();

        return 0U;
    }

    address  = SETTINGS_FLASH_ADDR;
    halfword = (const uint16_t *)&image;

    for(i = 0U; i < (sizeof(image) / sizeof(uint16_t)); i++)
    {
        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
                             address,
                             halfword[i]) != HAL_OK)
        {
            HAL_FLASH_Lock();

            return 0U;
        }

        address += sizeof(uint16_t);
    }

    HAL_FLASH_Lock();

    return 1U;
}

/*=============================================================
 * Инициализация: Flash или значения по умолчанию
 *=============================================================*/

void SETTINGS_Init(void)
{
    if(!SETTINGS_Load())
    {
        SETTINGS_SetDefaults();
    }

    SETTINGS_Apply();
}

void SETTINGS_SetDefaults(void)
{
    Settings.PulseWidthUs   = 100U;
    Settings.WorkMode       = WORK_MODE_CONTINUOUS;
    Settings.DurationMs     = 500U;

    Settings.ControlMode    = CONTROL_MODE_LOCAL;

    Settings.Rs485BaudIndex = 0U;      /* 9600 */
    Settings.Rs485Address   = 1U;
    Settings.Rs485Parity    = RS485_PARITY_NONE;
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

void SETTINGS_RequestSave(void)
{
    SaveRequested = 1U;
}

void SETTINGS_Process(void)
{
    if(!SaveRequested)
    {
        return;
    }

    if(Device_GetState() != DEVICE_READY)
    {
        return;
    }

    SaveRequested = 0U;

    (void)SETTINGS_Save();
}

uint32_t SETTINGS_GetRs485Baud(void)
{
    if(Settings.Rs485BaudIndex >= SETTINGS_RS485_BAUD_COUNT)
    {
        return SETTINGS_Rs485BaudTable[0];
    }

    return SETTINGS_Rs485BaudTable[Settings.Rs485BaudIndex];
}
