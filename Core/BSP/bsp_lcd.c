#include "bsp_lcd.h"

#include "st7789.h"
#include "fonts.h"


#include <stdio.h>
#include <string.h>

/*----------------------------------------------------------
    LCD state
----------------------------------------------------------*/

typedef struct
{
    uint16_t Angle;

    uint8_t SyncOK;

    uint8_t RS485OK;

    uint8_t Remote;

    uint16_t Duration;

    uint8_t Heartbeat;

    char Status[20];

    uint16_t StatusColor;

} LCD_State_t;

/*----------------------------------------------------------
    Static variables
----------------------------------------------------------*/

static LCD_Screen_t CurrentScreen = LCD_SCREEN_SPLASH;

static LCD_State_t LCD_State;

/*----------------------------------------------------------
    Private functions
----------------------------------------------------------*/

static void LCD_DrawSplash(void);
static void LCD_DrawMain(void);
static void LCD_DrawMenu(void);
static void LCD_DrawSettings(void);
static void LCD_DrawService(void);

static void LCD_DrawIcon(uint16_t x, uint16_t y, uint8_t ok);

/*==========================================================
    BSP_LCD_Init
==========================================================*/

void BSP_LCD_Init(void)
{
    ST7789_Init();

    BSP_LCD_Invalidate();

    CurrentScreen = LCD_SCREEN_SPLASH;
}

/*==========================================================
    BSP_LCD_Clear
==========================================================*/

void BSP_LCD_Clear(void)
{
    Watchdog_Refresh();

    ST7789_Fill_Color_DMA(
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    BSP_LCD_Invalidate
==========================================================*/

void BSP_LCD_Invalidate(void)
{
    memset(&LCD_State,0xFF,sizeof(LCD_State));

    /* strcmp() по Status требует завершающего нуля */
    LCD_State.Status[sizeof(LCD_State.Status)-1] = '\0';
}

/*==========================================================
    BSP_LCD_SetScreen
==========================================================*/

void BSP_LCD_SetScreen(
        LCD_Screen_t screen)
{
    if(CurrentScreen == screen)
    {
        return;
    }

    CurrentScreen = screen;

    BSP_LCD_Clear();

    switch(CurrentScreen)
    {
        case LCD_SCREEN_SPLASH:

            LCD_DrawSplash();
            break;

        case LCD_SCREEN_MAIN:

            LCD_DrawMain();
            break;

        case LCD_SCREEN_MENU:

            LCD_DrawMenu();
            break;

        case LCD_SCREEN_SETTINGS:

            LCD_DrawSettings();
            break;

        case LCD_SCREEN_SERVICE:

            LCD_DrawService();
            break;

        default:
            break;
    }
}

/*==========================================================
    BSP_LCD_GetScreen
==========================================================*/

LCD_Screen_t BSP_LCD_GetScreen(void)
{
    return CurrentScreen;
}

/*==========================================================
    BSP_LCD_Update
==========================================================*/

void BSP_LCD_Update(void)
{

}

/*==========================================================
    LCD_DrawSplash
==========================================================*/

static void LCD_DrawSplash(void)
{
    ST7789_WriteString(
            55,
            30,
            "THYRISTOR",
            Font_11x18,
            UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            45,
            55,
            "CONTROLLER",
            Font_11x18,
            UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);

    LCD_DrawIcon(104, 90, 1U);

    ST7789_WriteString(
            82,
            140,
            "READY",
            Font_11x18,
            UI_COLOR_OK,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    LCD_DrawMain
==========================================================*/

static void LCD_DrawMain(void)
{
    /*------------------------------------------------------
        Background
    ------------------------------------------------------*/

    BSP_LCD_Clear();

    /*------------------------------------------------------
        Horizontal lines
    ------------------------------------------------------*/

    ST7789_DrawLine(0,40,239,40,UI_COLOR_BORDER);

    ST7789_DrawLine(0,80,239,80,UI_COLOR_BORDER);

    ST7789_DrawLine(0,185,239,185,UI_COLOR_BORDER);

    ST7789_DrawLine(0,275,239,275,UI_COLOR_BORDER);

    /*------------------------------------------------------
        Status rectangle
    ------------------------------------------------------*/

    ST7789_DrawRectangle(
            20,
            195,
            200,
            42,
            UI_COLOR_BORDER);

    /*------------------------------------------------------
        Static text
    ------------------------------------------------------*/

    ST7789_WriteString(
            8,
            10,
            "LOCAL",
            Font_11x18,
            UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            120,
            10,
            "RS485:",
            Font_11x18,
            UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            8,
            50,
            "SYNC:",
            Font_11x18,
            UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            68,
            150,
            "DEGREES",
            Font_11x18,
            UI_COLOR_LABEL,
            UI_COLOR_BACKGROUND);

    /*------------------------------------------------------
        Draw dynamic fields
    ------------------------------------------------------*/

    BSP_LCD_UpdateMode(LCD_State.Remote);

    BSP_LCD_UpdateRS485(LCD_State.RS485OK);

    BSP_LCD_UpdateSync(LCD_State.SyncOK);

    BSP_LCD_UpdateAngle(LCD_State.Angle);

    BSP_LCD_UpdateStatus(
            LCD_State.Status,
            LCD_State.StatusColor);

    BSP_LCD_UpdateDuration(LCD_State.Duration);

    BSP_LCD_UpdateHeartbeat();
}

/*==========================================================
    LCD_DrawMenu
==========================================================*/

static void LCD_DrawMenu(void)
{
    BSP_LCD_Clear();

    ST7789_WriteString(
            80,
            20,
            "MENU",
            Font_16x26,
            UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    LCD_DrawSettings
==========================================================*/

static void LCD_DrawSettings(void)
{
    BSP_LCD_Clear();

    ST7789_WriteString(
            45,
            20,
            "SETTINGS",
            Font_16x26,
            UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    LCD_DrawService
==========================================================*/

static void LCD_DrawService(void)
{
    BSP_LCD_Clear();

    ST7789_WriteString(
            55,
            20,
            "SERVICE",
            Font_16x26,
            UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    BSP_LCD_UpdateMode
==========================================================*/

void BSP_LCD_UpdateMode(uint8_t remote)
{
    if(LCD_State.Remote == remote)
    {
        return;
    }

    LCD_State.Remote = remote;

    ST7789_FillRectangle(
            0,
            0,
            110,
            40,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            8,
            10,
            remote ? "REMOTE" : "LOCAL",
            Font_11x18,
            remote ? UI_COLOR_WARNING : UI_COLOR_TITLE,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    BSP_LCD_UpdateRS485
==========================================================*/

void BSP_LCD_UpdateRS485(uint8_t ok)
{
    if(LCD_State.RS485OK == ok)
    {
        return;
    }

    LCD_State.RS485OK = ok;

    ST7789_FillRectangle(
            185,
            4,
            40,
            32,
            UI_COLOR_BACKGROUND);

    LCD_DrawIcon(190, 4, ok);
}

/*==========================================================
    BSP_LCD_UpdateSync
==========================================================*/

void BSP_LCD_UpdateSync(uint8_t ok)
{
    if(LCD_State.SyncOK == ok)
    {
        return;
    }

    LCD_State.SyncOK = ok;

    ST7789_FillRectangle(
            70,
            44,
            40,
            32,
            UI_COLOR_BACKGROUND);

    LCD_DrawIcon(75, 44, ok);
}

/*==========================================================
    BSP_LCD_UpdateAngle
==========================================================*/

void BSP_LCD_UpdateAngle(uint16_t angle)
{
    char text[8];

    if(LCD_State.Angle == angle)
    {
        return;
    }

    LCD_State.Angle = angle;

    sprintf(text,"%3u",angle);

    ST7789_FillRectangle(
            45,
            95,
            150,
            48,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            60,
            100,
            text,
            Font_16x26,
            UI_COLOR_VALUE,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    BSP_LCD_UpdateStatus
==========================================================*/

void BSP_LCD_UpdateStatus(const char *text,
                          uint16_t color)
{
	if((strcmp(LCD_State.Status, text) == 0) &&
	   (LCD_State.StatusColor == color))
	{
	    return;
	}
	strncpy(LCD_State.Status,
	        text,
	        sizeof(LCD_State.Status)-1);

	LCD_State.Status[sizeof(LCD_State.Status)-1] = '\0';

	LCD_State.StatusColor = color;

	ST7789_FillRectangle(
            22,
            197,
            196,
            38,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            55,
            207,
            text,
            Font_11x18,
            color,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    BSP_LCD_UpdateDuration
==========================================================*/

void BSP_LCD_UpdateDuration(uint16_t ms)
{
    char text[16];

    if(LCD_State.Duration == ms)
    {
        return;
    }

    LCD_State.Duration = ms;

    sprintf(text,"%u ms",ms);

    ST7789_FillRectangle(
            10,
            245,
            120,
            22,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            10,
            250,
            text,
            Font_7x10,
            UI_COLOR_LABEL,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    LCD_DrawIcon

    Иконки рисуются примитивами, а не картинкой: фон всегда совпадает
    с фоном экрана, цвета задаются в ui_colors.h.
==========================================================*/

#define LCD_ICON_SIZE     32U
#define LCD_ICON_RADIUS   15

static void LCD_FillCircle(int16_t cx, int16_t cy, int16_t r, uint16_t color)
{
    int16_t dy;

    for(dy = -r; dy <= r; dy++)
    {
        int16_t dx = 0;

        while((dx * dx + dy * dy) <= (r * r))
        {
            dx++;
        }

        dx--;

        ST7789_FillRectangle((uint16_t)(cx - dx),
                             (uint16_t)(cy + dy),
                             (uint16_t)(2 * dx + 1),
                             1U,
                             color);
    }
}

/* Толстая линия из нескольких смежных отрезков */
static void LCD_DrawThickLine(int16_t x0, int16_t y0,
                              int16_t x1, int16_t y1,
                              uint16_t color)
{
    int16_t i;

    for(i = -1; i <= 1; i++)
    {
        ST7789_DrawLine((uint16_t)(x0 + i), (uint16_t)y0,
                        (uint16_t)(x1 + i), (uint16_t)y1, color);

        ST7789_DrawLine((uint16_t)x0, (uint16_t)(y0 + i),
                        (uint16_t)x1, (uint16_t)(y1 + i), color);
    }
}

static void LCD_DrawIcon(uint16_t x, uint16_t y, uint8_t ok)
{
    const int16_t cx = (int16_t)x + (LCD_ICON_SIZE / 2);
    const int16_t cy = (int16_t)y + (LCD_ICON_SIZE / 2);

    ST7789_FillRectangle(x, y, LCD_ICON_SIZE, LCD_ICON_SIZE,
                         UI_COLOR_BACKGROUND);

    LCD_FillCircle(cx, cy, LCD_ICON_RADIUS,
                   ok ? UI_COLOR_ICON_OK : UI_COLOR_ICON_ERROR);

    if(ok)
    {
        /* Галочка */
        LCD_DrawThickLine(cx - 7, cy,     cx - 2, cy + 6, UI_COLOR_ICON_GLYPH);
        LCD_DrawThickLine(cx - 2, cy + 6, cx + 7, cy - 6, UI_COLOR_ICON_GLYPH);
    }
    else
    {
        /* Крест */
        LCD_DrawThickLine(cx - 6, cy - 6, cx + 6, cy + 6, UI_COLOR_ICON_GLYPH);
        LCD_DrawThickLine(cx + 6, cy - 6, cx - 6, cy + 6, UI_COLOR_ICON_GLYPH);
    }
}

/*==========================================================
    BSP_LCD_UpdateDebug

    S - переходы через ноль, G - отбракованные (помеха),
    C - импульсы CH1, R - перезапуски до конца импульса (всё за 1 с)
==========================================================*/

void BSP_LCD_UpdateDebug(uint16_t sync,
                         uint16_t glitch,
                         uint16_t pulses,
                         uint16_t restarts,
                         uint16_t watchdog)
{
    char text[48];

    sprintf(text,"S%u G%u C%u R%u W%u",sync,glitch,pulses,restarts,watchdog);

    ST7789_FillRectangle(
            10,
            262,
            220,
            12,
            UI_COLOR_BACKGROUND);

    ST7789_WriteString(
            10,
            263,
            text,
            Font_7x10,
            UI_COLOR_LABEL,
            UI_COLOR_BACKGROUND);
}

/*==========================================================
    BSP_LCD_UpdateProgress
==========================================================*/

/*==========================================================
    BSP_LCD_UpdateHeartbeat

    Вращающийся индикатор: показывает, что главный цикл жив.
==========================================================*/

void BSP_LCD_UpdateHeartbeat(void)
{
    static const char Frames[] = { '|', '/', '-', '\\' };

    char text[2];

    LCD_State.Heartbeat = (uint8_t)((LCD_State.Heartbeat + 1U) & 0x03U);

    text[0] = Frames[LCD_State.Heartbeat];
    text[1] = '\0';

    ST7789_WriteString(
            112,
            282,
            text,
            Font_16x26,
            UI_COLOR_VALUE,
            UI_COLOR_BACKGROUND);
}

