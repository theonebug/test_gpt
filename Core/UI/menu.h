#ifndef UI_MENU_H_
#define UI_MENU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "event.h"

/*=============================================================
 * Типы пунктов меню
 *=============================================================*/
typedef enum
{
    /* Числовой параметр: Min..Max с шагом Step */
    MENU_ITEM_UINT = 0,

    /* Перечисление: значение - индекс в Options[] */
    MENU_ITEM_ENUM,

    /* Пункт-действие (без значения), например "EXIT" */
    MENU_ITEM_ACTION

} MenuItemType_t;

/*=============================================================
 * Описание пункта меню
 *
 * Добавление нового пункта = одна строка в таблице MenuItems
 * в menu.c. Value указывает на поле структуры Settings.
 *=============================================================*/
typedef struct
{
    const char        *Title;
    MenuItemType_t     Type;

    uint16_t          *Value;      /* NULL для MENU_ITEM_ACTION            */

    uint16_t           Min;
    uint16_t           Max;
    uint16_t           Step;

    const char        *Unit;       /* подпись единиц измерения, может NULL */

    const char *const *Options;    /* строки для MENU_ITEM_ENUM            */

    /* Своё форматирование значения (например 500 -> "0.5 s").
       Если NULL - используется формат по умолчанию. */
    void (*Format)(uint16_t value, char *buf, uint8_t size);

    /* Вызывается после изменения значения */
    void (*OnChange)(void);

    /* Вызывается для MENU_ITEM_ACTION */
    void (*OnAction)(void);

} MenuItem_t;

/*=============================================================
 * API
 *=============================================================*/
void MENU_Init(void);

/* Открыть/закрыть меню настроек */
void MENU_Open(void);
void MENU_Close(void);

bool MENU_IsActive(void);

/* Обработка события. Возвращает true, если событие поглощено меню. */
bool MENU_HandleEvent(const EventMessage_t *msg);

/* Полная перерисовка экрана меню */
void MENU_Draw(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_MENU_H_ */
