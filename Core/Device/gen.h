#ifndef DEVICE_GEN_H_
#define DEVICE_GEN_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=============================================================
 * Генератор прямоугольных импульсов для режима FIRST WAVE
 * Работает на TIM1 (свободный 16-битный таймер).
 * Формирует импульсы с заданной частотой и скважностью ~50%
 * на выходе TIRISTOR_OUT (PC14).
 *=============================================================*/

/* Инициализация TIM1 (вызывается один раз при старте) */
void GEN_Init(void);

/* Запуск генератора с заданной частотой (Гц).
   Импульс ~50% от периода, выводится на TIRISTOR_OUT. */
void GEN_Start(uint16_t freqHz);

/* Остановка генератора и сброс вывода в 0 */
void GEN_Stop(void);

/* 1 - генератор в данный момент работает */
uint8_t GEN_IsRunning(void);

/* Вызывать из фонового цикла (Device_Update) для контроля времени работы */
void GEN_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_GEN_H_ */
