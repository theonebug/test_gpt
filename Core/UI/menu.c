#include "menu.h"

#include <stdio.h>
#include <string.h>

#include "st7789.h"
#include "fonts.h"
#include "ui_colors.h"
#include "bsp_lcd.h"

#include "settings.h"
#include "device.h"

/*=============================================================
 * Геометрия экрана меню
 *=============================================================*/
#define MENU_TITLE_Y        10U
#define MENU_LIST_Y         50U
#define MENU_ROW_H          26U
#define MENU_ROW_TEXT_DY    4U
#define MENU_VISIBLE_ROWS   9U
#define MENU_VALUE_X        130U
#define MENU_TEXT_X         8U

/*=============================================================
 * Форматирование отдельных значений
 *=============================================================*/

static void MENU_FormatDuration(uint16_t value, char *buf, uint8_t size)
{
    /* 100..1000 мс -> "0.1 s" .. "1.0 s" */
    snprintf(buf, size, "%u.%u s", value / 1000U, (value % 1000U) / 100U);
}

static void MENU_FormatBaud(uint16_t value, char *buf, uint8_t size)
{
    uint16_t index = (value < SETTINGS_RS485_BAUD_COUNT) ? value : 0U;

    snprintf(buf, size, "%lu", (unsigned long)SETTINGS_Rs485BaudTable[index]);
}

/*=============================================================
 * Строки перечислений
 *=============================================================*/

static const char *const MENU_WorkModeNames[] =
{
    "CONTINUOUS",
    "FIRST WAVE",
    "TIMED"
};

static const char *const MENU_ControlNames[] =
{
    "LOCAL",
    "REMOTE"
};

static const char *const MENU_ParityNames[] =
{
    "NONE",
    "EVEN",
    "ODD"
};

/*=============================================================
 * Действия
 *=============================================================*/

static void MENU_ActionExit(void)
{
    MENU_Close();
}

/*=============================================================
 * Таблица пунктов меню
 *
 * Новый пункт добавляется одной строкой: указатель на поле
 * Settings + границы (или список строк для ENUM).
 *=============================================================*/
static const MenuItem_t MenuItems[] =
{
    {
        "PULSE",        MENU_ITEM_UINT,  &Settings.PulseWidthUs,
        SETTINGS_PULSE_WIDTH_MIN_US, SETTINGS_PULSE_WIDTH_MAX_US,
        SETTINGS_PULSE_WIDTH_STEP_US,
        "us",           NULL,            NULL, SETTINGS_Apply, NULL
    },
    {
        "MODE",         MENU_ITEM_ENUM,  &Settings.WorkMode,
        0U, WORK_MODE_COUNT - 1U, 1U,
        NULL,           MENU_WorkModeNames, NULL, SETTINGS_Apply, NULL
    },
    {
        "TIME",         MENU_ITEM_UINT,  &Settings.DurationMs,
        SETTINGS_DURATION_MIN_MS, SETTINGS_DURATION_MAX_MS,
        SETTINGS_DURATION_STEP_MS,
        NULL,           NULL,            MENU_FormatDuration, SETTINGS_Apply, NULL
    },
    {
        "CONTROL",      MENU_ITEM_ENUM,  &Settings.ControlMode,
        0U, CONTROL_MODE_COUNT - 1U, 1U,
        NULL,           MENU_ControlNames, NULL, SETTINGS_Apply, NULL
    },
    {
        "RS485 BAUD",   MENU_ITEM_UINT,  &Settings.Rs485BaudIndex,
        0U, SETTINGS_RS485_BAUD_COUNT - 1U, 1U,
        NULL,           NULL,            MENU_FormatBaud, SETTINGS_Apply, NULL
    },
    {
        "RS485 ADDR",   MENU_ITEM_UINT,  &Settings.Rs485Address,
        SETTINGS_RS485_ADDR_MIN, SETTINGS_RS485_ADDR_MAX, 1U,
        NULL,           NULL,            NULL, SETTINGS_Apply, NULL
    },
    {
        "RS485 PARITY", MENU_ITEM_ENUM,  &Settings.Rs485Parity,
        0U, RS485_PARITY_COUNT - 1U, 1U,
        NULL,           MENU_ParityNames, NULL, SETTINGS_Apply, NULL
    },
    {
        "EXIT",         MENU_ITEM_ACTION, NULL,
        0U, 0U, 0U,
        NULL,           NULL,            NULL, NULL, MENU_ActionExit
    }
};

#define MENU_ITEM_COUNT  (sizeof(MenuItems) / sizeof(MenuItems[0]))

/*=============================================================
 * Состояние меню
 *=============================================================*/
static bool    MenuActive  = false;
static bool    MenuEditing = false;
static uint8_t MenuCursor  = 0U;
static uint8_t MenuTop     = 0U;

/*=============================================================
 * Отрисовка
 *=============================================================*/

static void MENU_FormatValue(const MenuItem_t *item, char *buf, uint8_t size)
{
    uint16_t value;

    if(item->Type == MENU_ITEM_ACTION)
    {
        buf[0] = '\0';
        return;
    }

    value = *item->Value;

    if(item->Format != NULL)
    {
        item->Format(value, buf, size);
        return;
    }

    if((item->Type == MENU_ITEM_ENUM) && (item->Options != NULL))
    {
        snprintf(buf, size, "%s", item->Options[value]);
        return;
    }

    if(item->Unit != NULL)
    {
        snprintf(buf, size, "%u %s", value, item->Unit);
    }
    else
    {
        snprintf(buf, size, "%u", value);
    }
}

static void MENU_DrawRow(uint8_t index)
{
    const MenuItem_t *item;
    char     value[16];
    uint16_t y;
    uint16_t bg;
    uint16_t titleColor;
    uint16_t valueColor;
    bool     selected;

    if((index < MenuTop) || (index >= (MenuTop + MENU_VISIBLE_ROWS)))
    {
        return;
    }

    item     = &MenuItems[index];
    selected = (index == MenuCursor);
    y        = MENU_LIST_Y + ((uint16_t)(index - MenuTop) * MENU_ROW_H);

    bg         = selected ? UI_COLOR_PANEL : UI_COLOR_BACKGROUND;
    titleColor = selected ? UI_COLOR_VALUE : UI_COLOR_LABEL;
    valueColor = (selected && MenuEditing) ? UI_COLOR_WARNING : UI_COLOR_VALUE;

    ST7789_FillRectangle(0, y, ST7789_WIDTH, MENU_ROW_H, bg);

    ST7789_WriteString(MENU_TEXT_X,
                       y + MENU_ROW_TEXT_DY,
                       item->Title,
                       Font_11x18,
                       titleColor,
                       bg);

    MENU_FormatValue(item, value, sizeof(value));

    if(value[0] != '\0')
    {
        ST7789_WriteString(MENU_VALUE_X,
                           y + MENU_ROW_TEXT_DY,
                           value,
                           Font_11x18,
                           valueColor,
                           bg);
    }
}

void MENU_Draw(void)
{
    uint8_t i;

    BSP_LCD_Clear();

    ST7789_WriteString(45,
                       MENU_TITLE_Y,
                       "SETTINGS",
                       Font_16x26,
                       UI_COLOR_TITLE,
                       UI_COLOR_BACKGROUND);

    ST7789_DrawLine(0, 44, ST7789_WIDTH - 1, 44, UI_COLOR_BORDER);

    for(i = 0U; i < MENU_ITEM_COUNT; i++)
    {
        MENU_DrawRow(i);
    }

    ST7789_WriteString(MENU_TEXT_X,
                       302,
                       "LONG PRESS - BACK",
                       Font_7x10,
                       UI_COLOR_BORDER,
                       UI_COLOR_BACKGROUND);
}

/*=============================================================
 * Навигация
 *=============================================================*/

static void MENU_MoveCursor(int8_t delta)
{
    uint8_t prev = MenuCursor;
    uint8_t prevTop = MenuTop;

    if(delta > 0)
    {
        if(MenuCursor + 1U >= MENU_ITEM_COUNT)
        {
            return;
        }

        MenuCursor++;
    }
    else
    {
        if(MenuCursor == 0U)
        {
            return;
        }

        MenuCursor--;
    }

    if(MenuCursor < MenuTop)
    {
        MenuTop = MenuCursor;
    }
    else if(MenuCursor >= (MenuTop + MENU_VISIBLE_ROWS))
    {
        MenuTop = (uint8_t)(MenuCursor - MENU_VISIBLE_ROWS + 1U);
    }

    if(MenuTop != prevTop)
    {
        MENU_Draw();
    }
    else
    {
        MENU_DrawRow(prev);
        MENU_DrawRow(MenuCursor);
    }
}

static void MENU_ChangeValue(int8_t direction)
{
    const MenuItem_t *item = &MenuItems[MenuCursor];
    uint32_t value;

    if(item->Value == NULL)
    {
        return;
    }

    value = *item->Value;

    if(direction > 0)
    {
        value += item->Step;

        if(value > item->Max)
        {
            value = item->Max;
        }
    }
    else
    {
        if(value < ((uint32_t)item->Min + item->Step))
        {
            value = item->Min;
        }
        else
        {
            value -= item->Step;
        }
    }

    if(value == *item->Value)
    {
        return;
    }

    *item->Value = (uint16_t)value;

    if(item->OnChange != NULL)
    {
        item->OnChange();
    }

    MENU_DrawRow(MenuCursor);
}

/*=============================================================
 * API
 *=============================================================*/

void MENU_Init(void)
{
    MenuActive  = false;
    MenuEditing = false;
    MenuCursor  = 0U;
    MenuTop     = 0U;
}

void MENU_Open(void)
{
    if(MenuActive)
    {
        return;
    }

    MenuActive  = true;
    MenuEditing = false;
    MenuCursor  = 0U;
    MenuTop     = 0U;

    BSP_LCD_SetScreen(LCD_SCREEN_SETTINGS);

    MENU_Draw();
}

void MENU_Close(void)
{
    if(!MenuActive)
    {
        return;
    }

    MenuActive  = false;
    MenuEditing = false;

    /* Настройки уезжают во Flash при выходе из меню */
    SETTINGS_RequestSave();

    /* Сбрасываем кэш значений, иначе главный экран не перерисует поля */
    BSP_LCD_Invalidate();

    BSP_LCD_SetScreen(LCD_SCREEN_MAIN);
}

bool MENU_IsActive(void)
{
    return MenuActive;
}

bool MENU_HandleEvent(const EventMessage_t *msg)
{
    const MenuItem_t *item;

    if(!MenuActive)
    {
        return false;
    }

    switch(msg->Id)
    {
        case EVENT_ENCODER_LEFT:

            if(MenuEditing)
            {
                MENU_ChangeValue(-1);
            }
            else
            {
                MENU_MoveCursor(-1);
            }

            return true;

        case EVENT_ENCODER_RIGHT:

            if(MenuEditing)
            {
                MENU_ChangeValue(+1);
            }
            else
            {
                MENU_MoveCursor(+1);
            }

            return true;

        case EVENT_ENCODER_CLICK:

            item = &MenuItems[MenuCursor];

            if(item->Type == MENU_ITEM_ACTION)
            {
                if(item->OnAction != NULL)
                {
                    item->OnAction();
                }
            }
            else
            {
                MenuEditing = !MenuEditing;

                MENU_DrawRow(MenuCursor);
            }

            return true;

        case EVENT_ENCODER_LONG:

            if(MenuEditing)
            {
                MenuEditing = false;

                MENU_DrawRow(MenuCursor);
            }
            else
            {
                MENU_Close();
            }

            return true;

        default:
            break;
    }

    return false;
}
