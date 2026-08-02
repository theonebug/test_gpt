#include "modbus.h"

#include "main.h"
#include "usart.h"

#include "settings.h"
#include "device.h"
#include "sync.h"
#include "tiristor.h"

#define MODBUS_BUFFER_SIZE      64U

/* Пауза, после которой кадр считается завершённым (3.5 символа с запасом) */
#define MODBUS_FRAME_GAP_MS     4U

/* Связь считается живой, если обмен был не позже этого времени назад */
#define MODBUS_ONLINE_MS        2000U

#define MODBUS_FC_READ_HOLDING  0x03U
#define MODBUS_FC_READ_INPUT    0x04U
#define MODBUS_FC_WRITE_SINGLE  0x06U
#define MODBUS_FC_WRITE_MULTI   0x10U

#define MODBUS_EX_ILLEGAL_FUNC  0x01U
#define MODBUS_EX_ILLEGAL_ADDR  0x02U
#define MODBUS_EX_ILLEGAL_VALUE 0x03U
#define MODBUS_EX_DEVICE_FAIL   0x04U

#define MODBUS_FIRMWARE_VERSION 2U

static volatile uint8_t  RxBuffer[MODBUS_BUFFER_SIZE];
static volatile uint8_t  RxCount = 0U;
static volatile uint32_t RxLastTick = 0U;

static uint8_t  TxBuffer[MODBUS_BUFFER_SIZE];

static uint32_t LastExchangeTick = 0U;

/*=============================================================
 * CRC16 Modbus
 *=============================================================*/

static uint16_t MODBUS_Crc(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i;
    uint8_t  bit;

    for(i = 0U; i < length; i++)
    {
        crc ^= data[i];

        for(bit = 0U; bit < 8U; bit++)
        {
            if(crc & 1U)
            {
                crc = (uint16_t)((crc >> 1) ^ 0xA001U);
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/*=============================================================
 * Инициализация
 *=============================================================*/

void MODBUS_Init(void)
{
    RxCount    = 0U;
    RxLastTick = HAL_GetTick();

    MODBUS_ApplySettings();

    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

    HAL_NVIC_SetPriority(USART2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void MODBUS_ApplySettings(void)
{
    uint32_t baud = SETTINGS_Rs485BaudTable[
            (Settings.Rs485BaudIndex < SETTINGS_RS485_BAUD_COUNT)
                    ? Settings.Rs485BaudIndex : 0U];

    __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);

    huart2.Init.BaudRate = baud;

    /* Чётность занимает девятый бит, поэтому слово удлиняется до 9 бит */
    switch(Settings.Rs485Parity)
    {
        case RS485_PARITY_EVEN:

            huart2.Init.Parity     = UART_PARITY_EVEN;
            huart2.Init.WordLength = UART_WORDLENGTH_9B;
            break;

        case RS485_PARITY_ODD:

            huart2.Init.Parity     = UART_PARITY_ODD;
            huart2.Init.WordLength = UART_WORDLENGTH_9B;
            break;

        default:

            huart2.Init.Parity     = UART_PARITY_NONE;
            huart2.Init.WordLength = UART_WORDLENGTH_8B;
            break;
    }

    if(HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }

    RxCount = 0U;

    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
}

/*=============================================================
 * Приём
 *=============================================================*/

void MODBUS_RxIRQHandler(void)
{
    uint8_t data;

    if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE))
    {
        __HAL_UART_CLEAR_OREFLAG(&huart2);
    }

    if(!__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE))
    {
        return;
    }

    data = (uint8_t)(huart2.Instance->DR & 0xFFU);

    if(RxCount < MODBUS_BUFFER_SIZE)
    {
        RxBuffer[RxCount] = data;
        RxCount++;
    }

    RxLastTick = HAL_GetTick();
}

/*=============================================================
 * Доступ к регистрам
 *=============================================================*/

static uint16_t MODBUS_ReadHolding(uint16_t address)
{
    switch(address)
    {
        case 0U:  return Device_GetAngle();
        case 1U:  return Settings.PulseWidthUs;
        case 2U:  return Settings.WorkMode;
        case 3U:  return Settings.DurationMs;
        case 4U:  return Settings.MaxRunTimeS;
        case 5U:  return Settings.FreqDeviationX10;
        case 6U:  return Settings.ControlMode;
        case 7U:  return Settings.Rs485BaudIndex;
        case 8U:  return Settings.Rs485Address;
        case 9U:  return Settings.Rs485Parity;
        case 11U: return Settings.ZeroCrossOffsetUs;
        default:  return 0U;
    }
}

static uint16_t MODBUS_ReadInput(uint16_t address)
{
    switch(address)
    {
        case 0U:  return (uint16_t)Device_GetState();
        case 1U:  return SYNC_IsPresent();
        case 2U:  return (uint16_t)SYNC_GetFrequency_x10();
        case 3U:  return (uint16_t)SYNC_GetHalfPeriodUs();
        case 4U:  return Device_GetAngle();
        case 5U:  return (uint16_t)Device_GetElapsedMs();
        case 6U:  return (uint16_t)SYNC_GetCounter();
        case 7U:  return (uint16_t)SYNC_GetGlitchCounter();
        case 8U:  return (uint16_t)Tiristor_GetCH1Counter();
        case 9U:  return (uint16_t)Tiristor_GetCH2Counter();
        case 10U: return (uint16_t)Tiristor_GetWatchdogCounter();
        case 11U: return Tiristor_IsActive();
        case 12U: return MODBUS_FIRMWARE_VERSION;
        default:  return 0U;
    }
}

static uint8_t MODBUS_InRange(uint16_t value, uint16_t min, uint16_t max)
{
    return ((value >= min) && (value <= max)) ? 1U : 0U;
}

/* Запись настроек разрешена только в удалённом режиме и только в покое */
static uint8_t MODBUS_WriteAllowed(uint16_t address, uint16_t value)
{
    if(address == 10U)
    {
        /* Останов доступен всегда, остальные команды - как обычная запись */
        if(value == MODBUS_CMD_STOP)
        {
            return 1U;
        }
    }

    if(Settings.ControlMode != CONTROL_MODE_REMOTE)
    {
        return 0U;
    }

    if(!Device_IsIdle())
    {
        return 0U;
    }

    return 1U;
}

static uint8_t MODBUS_WriteHolding(uint16_t address, uint16_t value)
{
    switch(address)
    {
        case 0U:

            if(!MODBUS_InRange(value, DEVICE_ANGLE_MIN, DEVICE_ANGLE_MAX))
            {
                return MODBUS_EX_ILLEGAL_VALUE;
            }

            Device_SetAngle(value);
            return 0U;

        case 1U:

            if(!MODBUS_InRange(value,
                               SETTINGS_PULSE_WIDTH_MIN_US,
                               SETTINGS_PULSE_WIDTH_MAX_US))
            {
                return MODBUS_EX_ILLEGAL_VALUE;
            }

            Settings.PulseWidthUs = value;
            break;

        case 2U:

            if(value >= WORK_MODE_COUNT) { return MODBUS_EX_ILLEGAL_VALUE; }

            Settings.WorkMode = value;
            break;

        case 3U:

            if(!MODBUS_InRange(value,
                               SETTINGS_DURATION_MIN_MS,
                               SETTINGS_DURATION_MAX_MS))
            {
                return MODBUS_EX_ILLEGAL_VALUE;
            }

            Settings.DurationMs = value;
            break;

        case 4U:

            if(!MODBUS_InRange(value,
                               SETTINGS_MAX_RUN_MIN_S,
                               SETTINGS_MAX_RUN_MAX_S))
            {
                return MODBUS_EX_ILLEGAL_VALUE;
            }

            Settings.MaxRunTimeS = value;
            break;

        case 5U:

            if(!MODBUS_InRange(value,
                               SETTINGS_FREQ_DEV_MIN_X10,
                               SETTINGS_FREQ_DEV_MAX_X10))
            {
                return MODBUS_EX_ILLEGAL_VALUE;
            }

            Settings.FreqDeviationX10 = value;
            break;

        case 6U:

            if(value >= CONTROL_MODE_COUNT) { return MODBUS_EX_ILLEGAL_VALUE; }

            Settings.ControlMode = value;
            break;

        case 7U:

            if(value >= SETTINGS_RS485_BAUD_COUNT)
            {
                return MODBUS_EX_ILLEGAL_VALUE;
            }

            Settings.Rs485BaudIndex = value;
            break;

        case 8U:

            if(!MODBUS_InRange(value,
                               SETTINGS_RS485_ADDR_MIN,
                               SETTINGS_RS485_ADDR_MAX))
            {
                return MODBUS_EX_ILLEGAL_VALUE;
            }

            Settings.Rs485Address = value;
            break;

        case 9U:

            if(value >= RS485_PARITY_COUNT) { return MODBUS_EX_ILLEGAL_VALUE; }

            Settings.Rs485Parity = value;
            break;

        case 11U:

            if(!MODBUS_InRange(value,
                               SETTINGS_ZC_OFFSET_MIN_US,
                               SETTINGS_ZC_OFFSET_MAX_US))
            {
                return MODBUS_EX_ILLEGAL_VALUE;
            }

            Settings.ZeroCrossOffsetUs = value;
            break;

        case 10U:

            switch(value)
            {
                case MODBUS_CMD_START:

                    Device_Start();
                    return 0U;

                case MODBUS_CMD_STOP:

                    Device_Stop();
                    return 0U;

                case MODBUS_CMD_SAVE:

                    SETTINGS_RequestSave();
                    return 0U;

                default:
                    return MODBUS_EX_ILLEGAL_VALUE;
            }

        default:

            return MODBUS_EX_ILLEGAL_ADDR;
    }

    SETTINGS_Apply();

    return 0U;
}

/*=============================================================
 * Формирование ответа
 *=============================================================*/

static void MODBUS_Send(uint16_t length)
{
    uint16_t crc = MODBUS_Crc(TxBuffer, length);

    TxBuffer[length]      = (uint8_t)(crc & 0xFFU);
    TxBuffer[length + 1U] = (uint8_t)(crc >> 8);

    HAL_UART_Transmit(&huart2, TxBuffer, length + 2U, 100U);
}

static void MODBUS_SendException(uint8_t function, uint8_t code)
{
    TxBuffer[0] = (uint8_t)Settings.Rs485Address;
    TxBuffer[1] = (uint8_t)(function | 0x80U);
    TxBuffer[2] = code;

    MODBUS_Send(3U);
}

/*=============================================================
 * Обработка кадра
 *=============================================================*/

static void MODBUS_HandleFrame(const uint8_t *frame, uint16_t length)
{
    uint8_t  function;
    uint16_t address;
    uint16_t count;
    uint16_t crc;
    uint16_t i;

    if(length < 4U)
    {
        return;
    }

    crc = MODBUS_Crc(frame, length - 2U);

    if((frame[length - 2U] != (uint8_t)(crc & 0xFFU)) ||
       (frame[length - 1U] != (uint8_t)(crc >> 8)))
    {
        return;
    }

    if(frame[0] != (uint8_t)Settings.Rs485Address)
    {
        /* Широковещательный адрес 0 обслуживаем без ответа */
        if(frame[0] != 0U)
        {
            return;
        }
    }

    LastExchangeTick = HAL_GetTick();

    function = frame[1];
    address  = (uint16_t)((frame[2] << 8) | frame[3]);

    switch(function)
    {
        case MODBUS_FC_READ_HOLDING:
        case MODBUS_FC_READ_INPUT:
        {
            uint16_t limit = (function == MODBUS_FC_READ_HOLDING)
                    ? MODBUS_HOLDING_COUNT : MODBUS_INPUT_COUNT;

            if(length < 8U) { return; }

            count = (uint16_t)((frame[4] << 8) | frame[5]);

            if((count == 0U) || (count > 32U) ||
               ((address + count) > limit))
            {
                MODBUS_SendException(function, MODBUS_EX_ILLEGAL_ADDR);
                return;
            }

            TxBuffer[0] = frame[0];
            TxBuffer[1] = function;
            TxBuffer[2] = (uint8_t)(count * 2U);

            for(i = 0U; i < count; i++)
            {
                uint16_t value = (function == MODBUS_FC_READ_HOLDING)
                        ? MODBUS_ReadHolding(address + i)
                        : MODBUS_ReadInput(address + i);

                TxBuffer[3U + (i * 2U)]      = (uint8_t)(value >> 8);
                TxBuffer[3U + (i * 2U) + 1U] = (uint8_t)(value & 0xFFU);
            }

            MODBUS_Send((uint16_t)(3U + (count * 2U)));
            break;
        }

        case MODBUS_FC_WRITE_SINGLE:
        {
            uint16_t value;
            uint8_t  status;

            if(length < 8U) { return; }

            value = (uint16_t)((frame[4] << 8) | frame[5]);

            if(address >= MODBUS_HOLDING_COUNT)
            {
                MODBUS_SendException(function, MODBUS_EX_ILLEGAL_ADDR);
                return;
            }

            if(!MODBUS_WriteAllowed(address, value))
            {
                MODBUS_SendException(function, MODBUS_EX_DEVICE_FAIL);
                return;
            }

            status = MODBUS_WriteHolding(address, value);

            if(status != 0U)
            {
                MODBUS_SendException(function, status);
                return;
            }

            for(i = 0U; i < 6U; i++)
            {
                TxBuffer[i] = frame[i];
            }

            MODBUS_Send(6U);
            break;
        }

        case MODBUS_FC_WRITE_MULTI:
        {
            uint8_t status = 0U;

            if(length < 9U) { return; }

            count = (uint16_t)((frame[4] << 8) | frame[5]);

            if((count == 0U) || (count > 16U) ||
               ((address + count) > MODBUS_HOLDING_COUNT) ||
               (frame[6] != (uint8_t)(count * 2U)) ||
               (length < (9U + (count * 2U))))
            {
                MODBUS_SendException(function, MODBUS_EX_ILLEGAL_ADDR);
                return;
            }

            for(i = 0U; i < count; i++)
            {
                uint16_t value = (uint16_t)((frame[7U + (i * 2U)] << 8) |
                                             frame[7U + (i * 2U) + 1U]);

                if(!MODBUS_WriteAllowed(address + i, value))
                {
                    MODBUS_SendException(function, MODBUS_EX_DEVICE_FAIL);
                    return;
                }

                status = MODBUS_WriteHolding(address + i, value);

                if(status != 0U)
                {
                    MODBUS_SendException(function, status);
                    return;
                }
            }

            for(i = 0U; i < 6U; i++)
            {
                TxBuffer[i] = frame[i];
            }

            MODBUS_Send(6U);
            break;
        }

        default:

            MODBUS_SendException(function, MODBUS_EX_ILLEGAL_FUNC);
            break;
    }
}

/*=============================================================
 * Фоновая обработка
 *=============================================================*/

void MODBUS_Process(void)
{
    uint8_t  frame[MODBUS_BUFFER_SIZE];
    uint16_t length;
    uint16_t i;

    if(RxCount == 0U)
    {
        return;
    }

    if((HAL_GetTick() - RxLastTick) < MODBUS_FRAME_GAP_MS)
    {
        return;
    }

    __HAL_UART_DISABLE_IT(&huart2, UART_IT_RXNE);

    length = RxCount;

    for(i = 0U; i < length; i++)
    {
        frame[i] = RxBuffer[i];
    }

    RxCount = 0U;

    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

    MODBUS_HandleFrame(frame, length);
}

uint8_t MODBUS_IsOnline(void)
{
    if(LastExchangeTick == 0U)
    {
        return 0U;
    }

    return ((HAL_GetTick() - LastExchangeTick) < MODBUS_ONLINE_MS) ? 1U : 0U;
}
