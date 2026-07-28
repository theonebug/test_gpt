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

/*=============================================================
 * Параметры
 *=============================================================*/
void Tiristor_SetDelayUs(uint16_t delay);
uint16_t Tiristor_GetDelayUs(void);

void Tiristor_SetPulseWidthUs(uint16_t width);
uint16_t Tiristor_GetPulseWidthUs(void);

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

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_TIRISTOR_H_ */
