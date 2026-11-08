#ifndef DEVICE_TIRISTOR_H_
#define DEVICE_TIRISTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*=============================================================
 * Режим работы
 *=============================================================*/
typedef enum
{
    TIRISTOR_MODE_FIRST = 0,
    TIRISTOR_MODE_ALL
} TiristorMode_t;

/*=============================================================
 * Инициализация
 *=============================================================*/
void Tiristor_Init(void);

/*=============================================================
 * Управление
 *=============================================================*/
void Tiristor_Start(void);
void Tiristor_Stop(void);

uint8_t Tiristor_IsActive(void);

void Tiristor_SetMode(TiristorMode_t mode);
TiristorMode_t Tiristor_GetMode(void);

/* Режим "пропускание полуволны": тиристор открывается сразу после перехода
   через ноль (без среза по углу), т.е. волна проходит без искажений.
   Используется FIRST WAVE для 2-й и следующих полуволн. */
void Tiristor_SetPassThrough(uint8_t pass);
uint8_t Tiristor_GetPassThrough(void);

/*=============================================================
 * Параметры
 *=============================================================*/
void Tiristor_SetDelayUs(uint16_t delay);
uint16_t Tiristor_GetDelayUs(void);

void Tiristor_SetPulseWidthUs(uint16_t width);
uint16_t Tiristor_GetPulseWidthUs(void);

/* Компенсация опережения сигнала детектора нуля, мкс */
void Tiristor_SetZeroCrossOffsetUs(uint16_t offset);
uint16_t Tiristor_GetZeroCrossOffsetUs(void);

/* Минимальная задержка открытия для проходных полуволн (PassThrough), мкс.
   Используется вместо угла при Tiristor_SetPassThrough(1), т.е. в режиме
   WORK_MODE_FIRST_WAVE на 2-й и последующих полуволнах. */
void Tiristor_SetPassMinDelayUs(uint16_t delay);
uint16_t Tiristor_GetPassMinDelayUs(void);

/*=============================================================
 * Событие перехода через ноль
 *=============================================================*/
void Tiristor_OnZeroCross(void);

/*=============================================================
 * Обработчики Compare каналов TIM2
 *=============================================================*/
void Tiristor_Channel1_IRQHandler(void);
void Tiristor_Channel2_IRQHandler(void);

uint32_t Tiristor_GetCH1Counter(void);
uint32_t Tiristor_GetCH2Counter(void);
uint32_t Tiristor_GetRestartCounter(void);
uint32_t Tiristor_GetWatchdogCounter(void);

/* Немедленное гашение выхода, безопасно вызывать из любого контекста */
void Tiristor_EmergencyOff(void);

/* Сторож по CH2, вызывать из фонового цикла */
void Tiristor_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_TIRISTOR_H_ */
