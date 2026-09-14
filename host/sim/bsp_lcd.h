#ifndef HOST_BSP_LCD_H
#define HOST_BSP_LCD_H

/*
 * Хост-заглушка BSP_LCD для тестов логики. Экран на тестах не нужен:
 * методы записывают последнее вызванное значение в глобальные переменные,
 * чтобы тест мог их проверить. Заменяет Core/BSP/bsp_lcd.h через include path.
 */

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

void BSP_LCD_Init(void);
void BSP_LCD_Clear(void);
void BSP_LCD_Invalidate(void);
void BSP_LCD_SetScreen(LCD_Screen_t screen);
LCD_Screen_t BSP_LCD_GetScreen(void);
void BSP_LCD_Update(void);
void BSP_LCD_UpdateMode(uint8_t remote);
void BSP_LCD_UpdateRS485(uint8_t ok);
void BSP_LCD_UpdateSync(uint8_t ok);
void BSP_LCD_UpdateAngle(uint16_t angle);
void BSP_LCD_UpdateStatus(const char *text, uint16_t color);
void BSP_LCD_UpdateDuration(uint16_t ms);
void BSP_LCD_UpdateHeartbeat(void);
void BSP_LCD_UpdateDebug(uint16_t sync, uint16_t glitch,
                         uint16_t pulses, uint16_t restarts,
                         uint16_t watchdog);

/* Наблюдаемые значения (для проверки в тестах) */
extern LCD_Screen_t HOST_LCD_Screen;
extern uint16_t     HOST_LCD_LastAngle;
extern const char  *HOST_LCD_LastStatus;

#endif /* HOST_BSP_LCD_H */