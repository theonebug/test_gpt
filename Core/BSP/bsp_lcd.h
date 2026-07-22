#ifndef BSP_LCD_H
#define BSP_LCD_H

#include <stdint.h>
#include "ui_colors.h"

typedef enum
{
    LCD_SCREEN_SPLASH = 0,
    LCD_SCREEN_MAIN,
    LCD_SCREEN_MENU,
    LCD_SCREEN_SETTINGS,
    LCD_SCREEN_SERVICE

} LCD_Screen_t;


/*----------------------------------------------------------
    LCD
----------------------------------------------------------*/

void BSP_LCD_Init(void);

void BSP_LCD_Clear(void);

/*----------------------------------------------------------
    Screen manager
----------------------------------------------------------*/

void BSP_LCD_SetScreen(LCD_Screen_t screen);

LCD_Screen_t BSP_LCD_GetScreen(void);

void BSP_LCD_Update(void);

/*----------------------------------------------------------
    Main screen
----------------------------------------------------------*/

void BSP_LCD_UpdateMode(uint8_t remote);

void BSP_LCD_UpdateRS485(uint8_t ok);

void BSP_LCD_UpdateSync(uint8_t ok);

void BSP_LCD_UpdateAngle(uint16_t angle);

void BSP_LCD_UpdateStatus(const char *text,
                          uint16_t color);

void BSP_LCD_UpdateDuration(uint16_t ms);

void BSP_LCD_UpdateProgress(uint8_t percent);

#endif
