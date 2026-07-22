#ifndef __ST7789_H__
#define __ST7789_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"
#include "stdint.h"
#include "stdbool.h"
#include "fonts.h"


/*
=================================================
                DISPLAY CONFIG
=================================================
*/


#define ST7789_WIDTH       240
#define ST7789_HEIGHT      320

#define ST7789_BGR_MODE 0x00



/*
=================================================
                SPI SELECT
=================================================

Сейчас тестируем:

STM32F103C8T6
SPI1_TX
DMA1 Channel3


Для перехода на SPI2:

закомментировать SPI1
раскомментировать SPI2

*/


//#define ST7789_USE_SPI1
#define ST7789_USE_SPI2



/*
=================================================
                SPI HANDLE
=================================================

Создается в st7789.c:

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

*/


#ifdef ST7789_USE_SPI1

#define ST7789_SPI_HANDLE hspi1

#endif


#ifdef ST7789_USE_SPI2

#define ST7789_SPI_HANDLE hspi2

#endif



/*
=================================================
                GPIO CONFIG
=================================================

Измени под свою отладочную плату

*/


/* LCD CS */

#define ST7789_CS_PORT       GPIOB
#define ST7789_CS_PIN        GPIO_PIN_14


/* LCD DC */

#define ST7789_DC_PORT       GPIOB
#define ST7789_DC_PIN        GPIO_PIN_12


/* LCD RESET */

#define ST7789_RST_PORT      GPIOA
#define ST7789_RST_PIN       GPIO_PIN_15




/*
=================================================
                GPIO MACROS
=================================================
*/


#define ST7789_CS_LOW()      HAL_GPIO_WritePin( \
                              ST7789_CS_PORT, \
                              ST7789_CS_PIN, \
                              GPIO_PIN_RESET)


#define ST7789_CS_HIGH()     HAL_GPIO_WritePin( \
                              ST7789_CS_PORT, \
                              ST7789_CS_PIN, \
                              GPIO_PIN_SET)



#define ST7789_DC_LOW()      HAL_GPIO_WritePin( \
                              ST7789_DC_PORT, \
                              ST7789_DC_PIN, \
                              GPIO_PIN_RESET)


#define ST7789_DC_HIGH()     HAL_GPIO_WritePin( \
                              ST7789_DC_PORT, \
                              ST7789_DC_PIN, \
                              GPIO_PIN_SET)



#define ST7789_RST_LOW()     HAL_GPIO_WritePin( \
                              ST7789_RST_PORT, \
                              ST7789_RST_PIN, \
                              GPIO_PIN_RESET)


#define ST7789_RST_HIGH()    HAL_GPIO_WritePin( \
                              ST7789_RST_PORT, \
                              ST7789_RST_PIN, \
                              GPIO_PIN_SET)




/*
=================================================
                COLORS RGB565
=================================================
*/


#define ST7789_BLACK         0x0000
#define ST7789_WHITE         0xFFFF
#define ST7789_RED           0xF800
#define ST7789_GREEN         0x07E0
#define ST7789_BLUE          0x001F

#define ST7789_CYAN          0x07FF
#define ST7789_MAGENTA       0xF81F
#define ST7789_YELLOW        0xFFE0

#define ST7789_ORANGE        0xFD20
#define ST7789_GRAY          0x8410




/*
=================================================
                DMA BUFFER
=================================================

512 байт =
256 пикселей RGB565


Для STM32F103 достаточно.
Можно увеличить до 1024.

*/


#define ST7789_DMA_BUFFER_SIZE       (240 * 2)
#define ST7789_DMA_PIXELS            \
        (ST7789_DMA_BUFFER_SIZE / 2)

/*
=================================================
                OFSET
=================================================

Но некоторые ST7789 240×320 имеют смещение:

*/

#define ST7789_X_OFFSET 0
#define ST7789_Y_OFFSET 0

/*
=================================================
                FUNCTIONS
=================================================
*/


void ST7789_Init(void);


/* low level */

void ST7789_WriteCommand(uint8_t cmd);


void ST7789_WriteData(
        uint8_t *data,
        uint16_t size);



void ST7789_WriteData_DMA(
        uint8_t *data,
        uint16_t size);



void ST7789_Wait_DMA(void);



/* hardware */

void ST7789_Reset(void);



/* display */

void ST7789_SetRotation(
        uint8_t rotation);



void ST7789_SetAddressWindow(
        uint16_t x0,
        uint16_t y0,
        uint16_t x1,
        uint16_t y1);



/* drawing */

void ST7789_DrawPixel(
        uint16_t x,
        uint16_t y,
        uint16_t color);



void ST7789_Fill_Color(
        uint16_t color);



void ST7789_Fill_Color_DMA(
        uint16_t color);

void ST7789_DrawRectangle(
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t color);


void ST7789_FillRectangle(
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t color);


void ST7789_DrawLine(
        int x0,
        int y0,
        int x1,
        int y1,
        uint16_t color);

void ST7789_Test(void);

void ST7789_Color_Test2(void);



void ST7789_WriteChar(uint16_t x,
                      uint16_t y,
                      char ch,
                      FontDef font,
                      uint16_t color,
                      uint16_t bgcolor);

void ST7789_WriteString(uint16_t x,
                        uint16_t y,
                        const char *str,
                        FontDef font,
                        uint16_t color,
                        uint16_t bgcolor);

void ST7789_FillRectangle_DMA(uint16_t x,
                              uint16_t y,
                              uint16_t w,
                              uint16_t h,
                              uint16_t color);

void ST7789_DrawImage(uint16_t x,
                      uint16_t y,
                      uint16_t w,
                      uint16_t h,
                      const uint8_t *image);
/*
=================================================
                DMA CALLBACK
=================================================
*/


void ST7789_DMA_Callback(
        SPI_HandleTypeDef *hspi);



#ifdef __cplusplus
}
#endif


#endif
