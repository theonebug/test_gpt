#ifndef BSP_LCD_H
#define BSP_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "spi.h"

/*----------------------------------------------------------
 * LCD size
 *---------------------------------------------------------*/
#define LCD_WIDTH      240U
#define LCD_HEIGHT     320U

/*----------------------------------------------------------
 * Colors (RGB565)
 *---------------------------------------------------------*/
#define LCD_COLOR_BLACK      0x0000
#define LCD_COLOR_WHITE      0xFFFF
#define LCD_COLOR_RED        0xF800
#define LCD_COLOR_GREEN      0x07E0
#define LCD_COLOR_BLUE       0x001F
#define LCD_COLOR_YELLOW     0xFFE0
#define LCD_COLOR_CYAN       0x07FF
#define LCD_COLOR_MAGENTA    0xF81F

/*----------------------------------------------------------
 * Public functions
 *---------------------------------------------------------*/

void BSP_LCD_Init(void);

void BSP_LCD_Reset(void);

void BSP_LCD_WriteCommand(uint8_t cmd);

void BSP_LCD_WriteData(uint8_t data);

void BSP_LCD_WriteBuffer(const uint8_t *buffer, uint16_t length);

void BSP_LCD_SetWindow(uint16_t x0,
                       uint16_t y0,
                       uint16_t x1,
                       uint16_t y1);

void BSP_LCD_FillScreen(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
