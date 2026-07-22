/*
=================================================
                ST7789 DRIVER
                STM32F103 + HAL + DMA

                SPI1 DMA1 Channel3
                SPI2 DMA1 Channel5
=================================================
*/
#include "st7789.h"
#include <stdlib.h>

/*
=================================================
                EXTERNAL SPI
=================================================
*/
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
/*
=================================================
                SPI POINTER
=================================================
Позволяет переключать SPI1/SPI2
через st7789.h
*/
#ifdef ST7789_USE_SPI1
static SPI_HandleTypeDef *ST7789_SPI =
        &hspi1;
#endif

#ifdef ST7789_USE_SPI2
static SPI_HandleTypeDef *ST7789_SPI =
        &hspi2;
#endif
/*
=================================================
                DMA VARIABLES
=================================================
*/
static volatile uint8_t dma_complete = 0;
static uint8_t dma_buffer
[
        ST7789_DMA_BUFFER_SIZE
];

#define ST7789_CHAR_BUFFER_SIZE    (16 * 26 * 2)

static uint8_t st7789_char_buffer[ST7789_CHAR_BUFFER_SIZE];
/*
=================================================
                SELECT / UNSELECT
=================================================
*/
static void ST7789_Select(void)
{
    ST7789_CS_LOW();
}

static void ST7789_UnSelect(void)
{
    ST7789_CS_HIGH();
}
/*
=================================================
                RESET DISPLAY
=================================================
*/
void ST7789_Reset(void)
{

    ST7789_RST_HIGH();
    HAL_Delay(20);
    ST7789_RST_LOW();
    HAL_Delay(20);
    ST7789_RST_HIGH();
    HAL_Delay(120);

}
/*
=================================================
                SPI TRANSMIT NORMAL
=================================================
*/
static void ST7789_SPI_Transmit(
        uint8_t *data,
        uint16_t size)
{

    HAL_SPI_Transmit(
            ST7789_SPI,
            data,
            size,
            HAL_MAX_DELAY);

}
/*
=================================================
                SPI TRANSMIT DMA
=================================================
*/
void ST7789_WriteData_DMA(uint8_t *data,
                          uint16_t size)
{
    dma_complete = 0;

    HAL_SPI_Transmit_DMA(
            ST7789_SPI,
            data,
            size);

    while(dma_complete == 0)
    {
    }

    // Ждём полного освобождения SPI
    while(HAL_SPI_GetState(ST7789_SPI) != HAL_SPI_STATE_READY)
    {
    }
}
/*
=================================================
                WAIT DMA
=================================================
*/
void ST7789_Wait_DMA(void)
{

    while(
        HAL_SPI_GetState(ST7789_SPI)
        != HAL_SPI_STATE_READY)
    {

    }

}
/*
=================================================
                WRITE COMMAND
=================================================
*/
void ST7789_WriteCommand(
        uint8_t cmd)
{

    ST7789_DC_LOW();


    ST7789_Select();



    ST7789_SPI_Transmit(
            &cmd,
            1);
    ST7789_UnSelect();

}
/*
=================================================
                WRITE DATA
=================================================
*/
void ST7789_WriteData(
        uint8_t *data,
        uint16_t size)
{

    ST7789_DC_HIGH();
    ST7789_Select();
    ST7789_SPI_Transmit(
            data,
            size);
    ST7789_UnSelect();

}
/*
=================================================
                SET ADDRESS WINDOW
=================================================
*/
void ST7789_SetAddressWindow(
        uint16_t x0,
        uint16_t y0,
        uint16_t x1,
        uint16_t y1)
{
    uint8_t data[4];
    x0 += ST7789_X_OFFSET;
    x1 += ST7789_X_OFFSET;

    y0 += ST7789_Y_OFFSET;
    y1 += ST7789_Y_OFFSET;
    /*
    -----------------------
        Column address
    -----------------------
    */
    ST7789_WriteCommand(
            0x2A);
    data[0] = x0 >> 8;
    data[1] = x0 & 0xFF;
    data[2] = x1 >> 8;
    data[3] = x1 & 0xFF;
    ST7789_WriteData(
            data,
            4);
    /*
    -----------------------
        Row address
    -----------------------
    */
    ST7789_WriteCommand(
            0x2B);
    data[0] = y0 >> 8;
    data[1] = y0 & 0xFF;
    data[2] = y1 >> 8;
    data[3] = y1 & 0xFF;
    ST7789_WriteData(
            data,
            4);

    /*
    -----------------------
        Memory write
    -----------------------
    */
    ST7789_WriteCommand(
            0x2C);
}
/*
=================================================
                DMA CALLBACK
=================================================
*/
void ST7789_DMA_Callback(
        SPI_HandleTypeDef *hspi)
{

    if(hspi == ST7789_SPI)
    {
        dma_complete = 1;
    }

}
/*
=================================================
                HAL SPI CALLBACK
=================================================

Этот callback вызывается HAL после
окончания DMA передачи.

Если в проекте уже есть такой callback
в stm32f1xx_it.c или main.c,
эту часть объединить.

=================================================
*/
void HAL_SPI_TxCpltCallback(
        SPI_HandleTypeDef *hspi)
{

    ST7789_DMA_Callback(hspi);

}
/*
=================================================
                SET ROTATION (ИСПРАВЛЕНО)
=================================================
*/
void ST7789_SetRotation(uint8_t rotation)
{
    uint8_t madctl = 0;

    // Исправлено: добавлен BGR флаг
    switch(rotation)
    {
        case 0: madctl = 0x00 | ST7789_BGR_MODE; break;  // 0x08
        case 1: madctl = 0x60 | ST7789_BGR_MODE; break;  // 0x68
        case 2: madctl = 0xC0 | ST7789_BGR_MODE; break;  // 0xC8
        case 3: madctl = 0xA0 | ST7789_BGR_MODE; break;  // 0xA8
        default: madctl = 0x00 | ST7789_BGR_MODE; break;
    }

    ST7789_WriteCommand(0x36);
    ST7789_WriteData(&madctl, 1);
}

/*
=================================================
                DISPLAY INIT
=================================================
*/
void ST7789_Init(void)
{
    ST7789_Reset();
    /*
    Software reset
    */
    ST7789_WriteCommand(
            0x01);
    HAL_Delay(150);
    /*
    Sleep out
    */
    ST7789_WriteCommand(
            0x11);
    HAL_Delay(120);
    /*
    Color mode
    16 bit RGB565
    */
    ST7789_WriteCommand(
            0x3A);
    uint8_t data = 0x55;
    ST7789_WriteData(
            &data,
            1);

    /*
    Memory access control
    */
    ST7789_SetRotation(0);
    /*
    Porch setting
    */
    ST7789_WriteCommand(
            0xB2);
    uint8_t porch[] =
    {
        0x0C,
        0x0C,
        0x00,
        0x33,
        0x33
    };
    ST7789_WriteData(
            porch,
            sizeof(porch));
    /*
    Gate control
    */
    ST7789_WriteCommand(
            0xB7);
    data = 0x35;
    ST7789_WriteData(
            &data,
            1);
    /*
    VCOM setting
    */
    ST7789_WriteCommand(
            0xBB);
    data = 0x19;
    ST7789_WriteData(
            &data,
            1);
    /*
    LCM control
    */
    ST7789_WriteCommand(
            0xC0);
    data = 0x2C;
    ST7789_WriteData(
            &data,
            1);
    /*
    VDV VRH enable
    */
    ST7789_WriteCommand(
            0xC2);
    data = 0x01;
    ST7789_WriteData(
            &data,
            1);
    /*
    VRH setting
    */
    ST7789_WriteCommand(
            0xC3);
    data = 0x12;
    ST7789_WriteData(
            &data,
            1);
    /*
    VDV setting
    */
    ST7789_WriteCommand(
            0xC4);
    data = 0x20;
    ST7789_WriteData(
            &data,
            1);
    /*
    Frame rate
    */
    ST7789_WriteCommand(
            0xC6);
    data = 0x0F;
    ST7789_WriteData(
            &data,
            1);
    /*
    Power control
    */
    ST7789_WriteCommand(
            0xD0);
    uint8_t power[] =
    {
        0xA4,
        0xA1
    };
    ST7789_WriteData(
            power,
            2);
    /*
    Positive voltage gamma
    */
    ST7789_WriteCommand(
            0xE0);
    uint8_t gamma1[] =
    {
        0xD0,
        0x04,
        0x0D,
        0x11,
        0x13,
        0x2B,
        0x3F,
        0x54,
        0x4C,
        0x18,
        0x0D,
        0x0B,
        0x1F,
        0x23
    };
    ST7789_WriteData(
            gamma1,
            sizeof(gamma1));
    /*
    Negative voltage gamma
    */
    ST7789_WriteCommand(
            0xE1);
    uint8_t gamma2[] =
    {
        0xD0,
        0x04,
        0x0C,
        0x11,
        0x13,
        0x2C,
        0x3F,
        0x44,
        0x51,
        0x2F,
        0x1F,
        0x1F,
        0x20,
        0x23
    };
    ST7789_WriteData(
            gamma2,
            sizeof(gamma2));
    /*
    Display inversion ON
    */
    ST7789_WriteCommand(
            0x21);
    ST7789_Fill_Color_DMA(0x0016);
    /*
    Display ON
    */
    ST7789_WriteCommand(
            0x29);
    HAL_Delay(100);

}
/*
=================================================
                FILL COLOR DMA (ИСПРАВЛЕНО)
=================================================
*/
void ST7789_Fill_Color_DMA(uint16_t color)
{
    uint32_t pixels;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    // ИСПРАВЛЕНО: правильный порядок байт
    for(uint16_t i=0; i<ST7789_DMA_BUFFER_SIZE; i+=2)
    {
        dma_buffer[i]   = hi;   // Старший байт
        dma_buffer[i+1] = lo;   // Младший байт
    }

    ST7789_SetAddressWindow(0, 0, ST7789_WIDTH-1, ST7789_HEIGHT-1);
    ST7789_DC_HIGH();
    ST7789_Select();

    pixels = ST7789_WIDTH * ST7789_HEIGHT;
    while(pixels)
    {
        uint16_t block = (pixels > ST7789_DMA_PIXELS) ?
                         ST7789_DMA_PIXELS : pixels;

        dma_complete = 0;
        HAL_SPI_Transmit_DMA(ST7789_SPI, dma_buffer, block * 2);
        while(dma_complete == 0) {}
        pixels -= block;
    }
    ST7789_UnSelect();
}

/*
=================================================
                DRAW PIXEL (ИСПРАВЛЕНО)
=================================================
*/
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;

    ST7789_SetAddressWindow(x, y, x, y);
    uint8_t data[2];

    // ИСПРАВЛЕНО: правильный порядок байт
    data[0] = color >> 8;
    data[1] = color & 0xFF;

    ST7789_WriteData(data, 2);
}

/*
=================================================
                FILL RECTANGLE (ИСПРАВЛЕНО)
=================================================
*/
void ST7789_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if(x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    if((x+w-1) >= ST7789_WIDTH) w = ST7789_WIDTH - x;
    if((y+h-1) >= ST7789_HEIGHT) h = ST7789_HEIGHT - y;

    ST7789_SetAddressWindow(x, y, x+w-1, y+h-1);

    uint32_t pixels = w * h;
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    // ИСПРАВЛЕНО: правильный порядок байт
    for(uint16_t i=0; i<ST7789_DMA_BUFFER_SIZE; i+=2)
    {
        dma_buffer[i]   = hi;
        dma_buffer[i+1] = lo;
    }

    ST7789_DC_HIGH();
    ST7789_Select();

    while(pixels)
    {
        uint16_t block = (pixels > ST7789_DMA_PIXELS) ?
                         ST7789_DMA_PIXELS : pixels;

        dma_complete = 0;
        HAL_SPI_Transmit_DMA(ST7789_SPI, dma_buffer, block * 2);
        while(dma_complete == 0) {}
        pixels -= block;
    }
    ST7789_UnSelect();
}
/*
=================================================
                DRAW RECTANGLE
=================================================
*/
void ST7789_DrawRectangle(
        uint16_t x,
        uint16_t y,
        uint16_t w,
        uint16_t h,
        uint16_t color)
{
    ST7789_FillRectangle(
            x,
            y,
            w,
            1,
            color);
    ST7789_FillRectangle(
            x,
            y+h-1,
            w,
            1,
            color);
    ST7789_FillRectangle(
            x,
            y,
            1,
            h,
            color);
    ST7789_FillRectangle(
            x+w-1,
            y,
            1,
            h,
            color);
}
/*
=================================================
                DRAW LINE
=================================================

Bresenham algorithm

=================================================
*/
void ST7789_DrawLine(
        int x0,
        int y0,
        int x1,
        int y1,
        uint16_t color)
{
    int dx =
        abs(x1-x0);
    int sx =
        x0<x1 ? 1 : -1;
    int dy =
        -abs(y1-y0);
    int sy =
        y0<y1 ? 1 : -1;
    int err =
        dx+dy;
    while(1)
    {
        ST7789_DrawPixel(
                x0,
                y0,
                color);
        if(x0==x1 &&
           y0==y1)
        {
            break;
        }
        int e2 =
            2*err;
        if(e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }
        if(e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}


/*
=================================================
                DISPLAY TEST
=================================================

Тест ST7789 240x320

Проверяет:

- ориентацию
- цвета
- границы
- адресное окно

=================================================
*/



void ST7789_WriteChar(uint16_t x,
                      uint16_t y,
                      char ch,
                      FontDef font,
                      uint16_t color,
                      uint16_t bgcolor)
{
    if(ch < 32)
        return;

    if((x + font.width) > ST7789_WIDTH)
        return;

    if((y + font.height) > ST7789_HEIGHT)
        return;

    uint32_t index = 0;

    for(uint8_t row = 0; row < font.height; row++)
    {
        uint16_t line = font.data[(ch - 32) * font.height + row];

        for(uint8_t col = 0; col < font.width; col++)
        {
            uint16_t pixel;

            if((line << col) & 0x8000)
            {
                pixel = color;
            }
            else
            {
                pixel = bgcolor;
            }

            st7789_char_buffer[index++] = pixel >> 8;
            st7789_char_buffer[index++] = pixel & 0xFF;
        }
    }

    ST7789_SetAddressWindow(
            x,
            y,
            x + font.width - 1,
            y + font.height - 1);



    ST7789_DC_HIGH();
    ST7789_Select();

    ST7789_WriteData_DMA(
            st7789_char_buffer,
            index);

   // ST7789_SPI_Transmit(
    //        st7789_char_buffer,
    //        index);

    ST7789_UnSelect();
}

void ST7789_WriteString(uint16_t x,
                        uint16_t y,
                        const char *str,
                        FontDef font,
                        uint16_t color,
                        uint16_t bgcolor)
{
    while(*str)
    {
        if(x + font.width >= ST7789_WIDTH)
        {
            x = 0;
            y += font.height;

            if(y + font.height >= ST7789_HEIGHT)
                break;

            if(*str == ' ')
            {
                str++;
                continue;
            }
        }

        ST7789_WriteChar(
                x,
                y,
                *str,
                font,
                color,
                bgcolor);

        x += font.width;

        str++;
    }
}

void ST7789_FillRectangle_DMA(uint16_t x,
                              uint16_t y,
                              uint16_t w,
                              uint16_t h,
                              uint16_t color)
{
    if((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
        return;

    if((x + w) > ST7789_WIDTH)
        w = ST7789_WIDTH - x;

    if((y + h) > ST7789_HEIGHT)
        h = ST7789_HEIGHT - y;

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    /* Заполняем DMA-буфер цветом */
    for(uint16_t i = 0; i < ST7789_DMA_BUFFER_SIZE; i += 2)
    {
        dma_buffer[i]     = hi;
        dma_buffer[i + 1] = lo;
    }

    ST7789_SetAddressWindow(
            x,
            y,
            x + w - 1,
            y + h - 1);

    ST7789_DC_HIGH();
    ST7789_Select();

    uint32_t pixels = (uint32_t)w * h;

    while(pixels)
    {
        uint16_t block;

        if(pixels > ST7789_DMA_PIXELS)
        {
            block = ST7789_DMA_PIXELS;
        }
        else
        {
            block = pixels;
        }

        dma_complete = 0;

        HAL_SPI_Transmit_DMA(
                ST7789_SPI,
                dma_buffer,
                block * 2);

        while(dma_complete == 0)
        {
        }

        pixels -= block;
    }

    ST7789_UnSelect();
}

void ST7789_DrawImage(uint16_t x,
                      uint16_t y,
                      uint16_t w,
                      uint16_t h,
                      const uint8_t *image)
{
    if((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
        return;

    if((x + w) > ST7789_WIDTH)
        return;

    if((y + h) > ST7789_HEIGHT)
        return;

    ST7789_SetAddressWindow(
            x,
            y,
            x + w - 1,
            y + h - 1);

    ST7789_DC_HIGH();
    ST7789_Select();

    uint32_t bytes = (uint32_t)w * h * 2;

    while(bytes)
    {
        uint16_t block = (bytes > ST7789_DMA_BUFFER_SIZE) ?
                          ST7789_DMA_BUFFER_SIZE : bytes;

        dma_complete = 0;

        HAL_SPI_Transmit_DMA(
                ST7789_SPI,
                (uint8_t *)image,
                block);

        while(dma_complete == 0)
        {
        }

        image += block;
        bytes -= block;
    }

    ST7789_UnSelect();
}
