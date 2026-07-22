/******************************************************************************
 * @file    bsp_lcd.c
 * @brief   BSP LCD Driver
 ******************************************************************************/

#include "bsp_lcd.h"

/*==============================================================================
 * Private Functions
 *============================================================================*/

/*---------------------------------------------------------------------------
 * Chip Select
 *--------------------------------------------------------------------------*/
static inline void LCD_Select(void)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
}

static inline void LCD_Unselect(void)
{
    //HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

/*---------------------------------------------------------------------------
 * Data / Command
 *--------------------------------------------------------------------------*/
static inline void LCD_CommandMode(void)
{
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
}

static inline void LCD_DataMode(void)
{
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
}

/*---------------------------------------------------------------------------
 * Reset
 *--------------------------------------------------------------------------*/
static inline void LCD_ResetLow(void)
{
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
}

static inline void LCD_ResetHigh(void)
{
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
}

/*---------------------------------------------------------------------------
 * SPI transmit
 *--------------------------------------------------------------------------*/
static void LCD_Send(const uint8_t *data, uint16_t size)
{
    HAL_SPI_Transmit(&hspi2, (uint8_t *)data, size, HAL_MAX_DELAY);
}

/*---------------------------------------------------------------------------
 * Internal functions (without CS switching)
 *--------------------------------------------------------------------------*/
static void LCD_WriteCommand_NoCS(uint8_t cmd)
{
    LCD_CommandMode();
    LCD_Send(&cmd, 1);
}

static void LCD_WriteData_NoCS(uint8_t data)
{
    LCD_DataMode();
    LCD_Send(&data, 1);
}

static void LCD_WriteBuffer_NoCS(const uint8_t *buffer, uint16_t length)
{
    LCD_DataMode();
    LCD_Send(buffer, length);
}

/*==============================================================================
 * Public Functions
 *============================================================================*/

/******************************************************************************
 * Hardware Reset
 ******************************************************************************/
void BSP_LCD_Reset(void)
{
    LCD_Unselect();

    LCD_ResetHigh();
    HAL_Delay(5);

    LCD_ResetLow();
    HAL_Delay(20);

    LCD_ResetHigh();
    HAL_Delay(120);
}

/******************************************************************************
 * LCD Init
 ******************************************************************************/
/******************************************************************************
 * LCD Init
 ******************************************************************************/
void BSP_LCD_Init(void)
{
    BSP_LCD_Reset();

    LCD_Select();

    /*----------------------------------------------------------
     * Software Reset
     *---------------------------------------------------------*/
    LCD_WriteCommand_NoCS(LCD_CMD_SWRESET);
    HAL_Delay(150);

    /*----------------------------------------------------------
     * Sleep Out
     *---------------------------------------------------------*/
    LCD_WriteCommand_NoCS(LCD_CMD_SLPOUT);
    HAL_Delay(120);

    /*----------------------------------------------------------
     * Pixel Format = RGB565
     *---------------------------------------------------------*/
    LCD_WriteCommand_NoCS(LCD_CMD_COLMOD);
    LCD_WriteData_NoCS(0x55);

    HAL_Delay(10);

    /*----------------------------------------------------------
     * Memory Access Control
     * MX=0 MY=0 MV=0 RGB
     *---------------------------------------------------------*/
    LCD_WriteCommand_NoCS(LCD_CMD_MADCTL);
    LCD_WriteData_NoCS(0x00);

    /*----------------------------------------------------------
     * Normal Display Mode
     *---------------------------------------------------------*/
    LCD_WriteCommand_NoCS(0x13);

    /*----------------------------------------------------------
     * Display ON
     *---------------------------------------------------------*/
    LCD_WriteCommand_NoCS(LCD_CMD_DISPON);
    HAL_Delay(20);

    LCD_Unselect();
}

/******************************************************************************
 * Send Command
 ******************************************************************************/
void BSP_LCD_WriteCommand(uint8_t cmd)
{
    LCD_Select();

    LCD_WriteCommand_NoCS(cmd);

    LCD_Unselect();
}

/******************************************************************************
 * Send one data byte
 ******************************************************************************/
void BSP_LCD_WriteData(uint8_t data)
{
    LCD_Select();

    LCD_WriteData_NoCS(data);

    LCD_Unselect();
}

/******************************************************************************
 * Send data buffer
 ******************************************************************************/
void BSP_LCD_WriteBuffer(const uint8_t *buffer, uint16_t length)
{
    LCD_Select();

    LCD_WriteBuffer_NoCS(buffer, length);

    LCD_Unselect();
}

/******************************************************************************
 * Set active window
 ******************************************************************************/
void BSP_LCD_SetWindow(uint16_t x0,
                       uint16_t y0,
                       uint16_t x1,
                       uint16_t y1)
{
    uint8_t data[4];

    LCD_Select();

    LCD_WriteCommand_NoCS(LCD_CMD_CASET);

    data[0] = x0 >> 8;
    data[1] = x0;
    data[2] = x1 >> 8;
    data[3] = x1;

    LCD_WriteBuffer_NoCS(data, 4);

    LCD_WriteCommand_NoCS(LCD_CMD_RASET);

    data[0] = y0 >> 8;
    data[1] = y0;
    data[2] = y1 >> 8;
    data[3] = y1;

    LCD_WriteBuffer_NoCS(data, 4);

    LCD_WriteCommand_NoCS(LCD_CMD_RAMWR);

    LCD_Unselect();
}

/******************************************************************************
 * Fill Screen
 ******************************************************************************/
void BSP_LCD_FillScreen(uint16_t color)
{
    uint32_t i;
    uint8_t pixel[2];

    pixel[0] = color >> 8;
    pixel[1] = color;

    BSP_LCD_SetWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

    LCD_Select();

    LCD_DataMode();

    for(i = 0; i < (uint32_t)LCD_WIDTH * LCD_HEIGHT; i++)
    {
        LCD_Send(pixel, 2);
    }

    LCD_Unselect();
}
