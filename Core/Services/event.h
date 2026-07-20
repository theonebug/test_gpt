#ifndef EVENT_H
#define EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*---------------------------------------------------------------------------
 * Размер очереди событий
 *---------------------------------------------------------------------------*/
#define EVENT_QUEUE_SIZE    16U

/*---------------------------------------------------------------------------
 * Идентификаторы событий
 *---------------------------------------------------------------------------*/
typedef enum
{
    EVENT_NONE = 0,

    /* Кнопки */

    EVENT_RUN_CLICK,
    EVENT_RUN_LONG,

    EVENT_STOP_CLICK,
    EVENT_STOP_LONG,

    /* Энкодер */

    EVENT_ENCODER_LEFT,
    EVENT_ENCODER_RIGHT,

    EVENT_ENCODER_CLICK,
    EVENT_ENCODER_LONG,

    /* Система */

    EVENT_SYNC,

    EVENT_RS485_RX,
    EVENT_RS485_TX,

    EVENT_ERROR

} EventId_t;

/*---------------------------------------------------------------------------
 * Сообщение
 *---------------------------------------------------------------------------*/
typedef struct
{
    EventId_t Id;
    int16_t   Data;

} EventMessage_t;

/*---------------------------------------------------------------------------
 * API
 *---------------------------------------------------------------------------*/
void EVENT_Init(void);

bool EVENT_Push(EventId_t id, int16_t data);

bool EVENT_Get(EventMessage_t *msg);

void EVENT_Clear(void);

bool EVENT_IsAvailable(void);

uint8_t EVENT_Count(void);

#ifdef __cplusplus
}
#endif

#endif
