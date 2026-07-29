#ifndef DEVICE_SETTINGS_H_
#define DEVICE_SETTINGS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*=============================================================
 * Режим работы
 *=============================================================*/
typedef enum
{
    /* Непрерывное срезание синусоиды до нажатия STOP */
    WORK_MODE_CONTINUOUS = 0,

    /* Срезается только первая пришедшая полуволна,
       остальные идут без искажений в течение заданного времени */
    WORK_MODE_FIRST_WAVE,

    /* Непрерывное срезание в течение заданного времени */
    WORK_MODE_TIMED,

    WORK_MODE_COUNT

} WorkMode_t;

/*=============================================================
 * Режим управления
 *=============================================================*/
typedef enum
{
    CONTROL_MODE_LOCAL = 0,
    CONTROL_MODE_REMOTE,

    CONTROL_MODE_COUNT

} ControlMode_t;

/*=============================================================
 * Чётность RS485
 *=============================================================*/
typedef enum
{
    RS485_PARITY_NONE = 0,
    RS485_PARITY_EVEN,
    RS485_PARITY_ODD,

    RS485_PARITY_COUNT

} Rs485Parity_t;

/*=============================================================
 * Границы параметров
 *=============================================================*/
#define SETTINGS_PULSE_WIDTH_MIN_US     50U
#define SETTINGS_PULSE_WIDTH_MAX_US     500U
#define SETTINGS_PULSE_WIDTH_STEP_US    10U

#define SETTINGS_DURATION_MIN_MS        100U
#define SETTINGS_DURATION_MAX_MS        1000U
#define SETTINGS_DURATION_STEP_MS       100U

#define SETTINGS_RS485_ADDR_MIN         1U
#define SETTINGS_RS485_ADDR_MAX         247U

/*=============================================================
 * Настройки прибора
 *=============================================================*/
typedef struct
{
    uint16_t PulseWidthUs;      /* длительность управляющего импульса   */
    uint16_t WorkMode;          /* WorkMode_t                           */
    uint16_t DurationMs;        /* время работы для режимов с таймером  */

    uint16_t ControlMode;       /* ControlMode_t                        */

    uint16_t Rs485BaudIndex;    /* индекс в SETTINGS_Rs485BaudTable     */
    uint16_t Rs485Address;      /* адрес в сети                         */
    uint16_t Rs485Parity;       /* Rs485Parity_t                        */

} Settings_t;

extern Settings_t Settings;

/* Таблица скоростей RS485, индексируется Settings.Rs485BaudIndex */
extern const uint32_t SETTINGS_Rs485BaudTable[];
#define SETTINGS_RS485_BAUD_COUNT   5U

/*=============================================================
 * API
 *=============================================================*/
/* Загружает настройки из Flash, иначе ставит значения по умолчанию */
void SETTINGS_Init(void);

void SETTINGS_SetDefaults(void);

/* Запись настроек во Flash. 1 - успешно (или изменений не было) */
uint8_t SETTINGS_Save(void);

/* Отложенное сохранение: запись выполнится в SETTINGS_Process(),
   когда прибор не в работе (стирание Flash стопорит ядро) */
void SETTINGS_RequestSave(void);

void SETTINGS_Process(void);

/* Перенос настроек в модули (тиристор, RS485 и т.д.).
   Вызывается после любого изменения настроек. */
void SETTINGS_Apply(void);

uint32_t SETTINGS_GetRs485Baud(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_SETTINGS_H_ */
