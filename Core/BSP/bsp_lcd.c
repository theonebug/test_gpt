#include "bsp_lcd.h"

#include "st7789.h"
#include "fonts.h"

#include "ok_32.h"
#include "error_32.h"

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

    uint8_t Progress;

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

    ST7789_DrawImage(
            104,
            90,
            32,
            32,
            ok_32);

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

    BSP_LCD_UpdateProgress(LCD_State.Progress);
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

    ST7789_DrawImage(
            190,
            4,
            32,
            32,
            ok ? ok_32 : error_32);
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

    ST7789_DrawImage(
            75,
            44,
            32,
            32,
            ok ? ok_32 : error_32);
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
    BSP_LCD_UpdateProgress
==========================================================*/

void BSP_LCD_UpdateProgress(uint8_t percent)
{
    uint16_t width;

    if(percent > 100)
    {
        percent = 100;
    }

    if(LCD_State.Progress == percent)
    {
        return;
    }

    LCD_State.Progress = percent;

    width = (200 * percent) / 100;

    ST7789_DrawRectangle(
            18,
            282,
            204,
            18,
            UI_COLOR_BORDER);

    ST7789_FillRectangle(
            20,
            284,
            200,
            14,
            UI_COLOR_BACKGROUND);

    ST7789_FillRectangle(
            20,
            284,
            width,
            14,
            UI_COLOR_OK);
}

