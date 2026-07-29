#ifndef SERVICES_MODBUS_H_
#define SERVICES_MODBUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*=============================================================
 * Modbus RTU slave на USART2 (модуль XY-S485, DE не используется)
 *
 * Holding registers (0x03 / 0x06 / 0x10):
 *   0  угол, градусы
 *   1  длительность импульса, мкс
 *   2  режим работы
 *   3  время работы, мс
 *   4  аварийный таймаут, с
 *   5  допуск частоты, 0.1 Гц
 *   6  режим управления (0 - local, 1 - remote)
 *   7  индекс скорости RS485
 *   8  адрес RS485
 *   9  чётность RS485
 *   10 команда: 1 - START, 2 - STOP, 3 - сохранить настройки
 *
 * Input registers (0x04):
 *   0  состояние устройства
 *   1  синхронизация есть
 *   2  частота сети, 0.1 Гц
 *   3  полупериод, мкс
 *   4  угол, градусы
 *   5  время с начала работы, мс
 *   6  переходы через ноль
 *   7  отбракованные события
 *   8  импульсы CH1
 *   9  импульсы CH2
 *   10 срабатывания сторожа CH2
 *   11 тиристор активен
 *   12 версия прошивки
 *=============================================================*/

#define MODBUS_HOLDING_COUNT    11U
#define MODBUS_INPUT_COUNT      13U

#define MODBUS_CMD_START        1U
#define MODBUS_CMD_STOP         2U
#define MODBUS_CMD_SAVE         3U

void MODBUS_Init(void);

/* Применяет скорость, чётность и адрес из настроек */
void MODBUS_ApplySettings(void);

/* Разбор принятого кадра, вызывать из главного цикла */
void MODBUS_Process(void);

/* Приём байта, вызывается из обработчика прерывания USART2 */
void MODBUS_RxIRQHandler(void);

/* Был ли обмен за последнюю секунду */
uint8_t MODBUS_IsOnline(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVICES_MODBUS_H_ */
