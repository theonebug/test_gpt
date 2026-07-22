#ifndef BSP_LCD_H
#define BSP_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "spi.h"

/*--------------------------------------------------------------------
 * LCD parameters
 *-------------------------------------------------------------------*/
#define LCD_WIDTH      240U
#define LCD_HEIGHT     320U

/*--------------------------------------------------------------------
 * RGB565 colors
 *-------------------------------------------------------------------*/
#define LCD_BLACK      0x0000
#define LCD_WHITE      0xFFFF
#define LCD_RED        0xF800
#define LCD_GREEN      0x07E0
#define LCD_BLUE       0x001F
#define LCD_YELLOW     0xFFE0
#define LCD_CYAN       0x07FF
#define LCD_MAGENTA    0xF81F

/*--------------------------------------------------------------------
 * ST7789 commands
 *-------------------------------------------------------------------*/
#define LCD_CMD_SWRESET    0x01
#define LCD_CMD_SLPOUT     0x11
#define LCD_CMD_DISPON     0x29
#define LCD_CMD_CASET      0x2A
#define LCD_CMD_RASET      0x2B
#define LCD_CMD_RAMWR      0x2C
#define LCD_CMD_MADCTL     0x36
#define LCD_CMD_COLMOD     0x3A

/*--------------------------------------------------------------------
 * Public interface
 *-------------------------------------------------------------------*/

void BSP_LCD_Init(void);

void BSP_LCD_Reset(void);

void BSP_LCD_WriteCommand(uint8_t cmd);

void BSP_LCD_WriteData(uint8_t data);

void BSP_LCD_WriteBuffer(const uint8_t *buffer,
                         uint16_t length);

void BSP_LCD_SetWindow(uint16_t x0,
                       uint16_t y0,
                       uint16_t x1,
                       uint16_t y1);

void BSP_LCD_FillScreen(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
