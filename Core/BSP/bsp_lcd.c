/******************************************************************************
 * @file    bsp_lcd.c
 * @brief   LCD BSP Driver (Low Level)
 ******************************************************************************/

#include "bsp_lcd.h"

/*=============================================================================
 * Private Functions
 *============================================================================*/

/*----------------------------------------------------------
 * Chip Select
 *---------------------------------------------------------*/
static void LCD_CS_Low(void)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
}

static void LCD_CS_High(void)
{
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

/*----------------------------------------------------------
 * Data / Command
 *---------------------------------------------------------*/
static void LCD_DC_Command(void)
{
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
}

static void LCD_DC_Data(void)
{
    HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
}

/*----------------------------------------------------------
 * Reset
 *---------------------------------------------------------*/
static void LCD_RST_Low(void)
{
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
}

static void LCD_RST_High(void)
{
    HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
}

/*----------------------------------------------------------
 * Send one byte
 *---------------------------------------------------------*/
static void LCD_SendByte(uint8_t data)
{
    HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
}

/*=============================================================================
 * Public Functions
 *============================================================================*/

/******************************************************************************
 * @brief LCD Hardware Reset
 ******************************************************************************/
void BSP_LCD_Reset(void)
{
    LCD_CS_High();

    LCD_RST_High();
    HAL_Delay(5);

    LCD_RST_Low();
    HAL_Delay(20);

    LCD_RST_High();
    HAL_Delay(120);
}

/******************************************************************************
 * @brief LCD Initialization
 ******************************************************************************/
void BSP_LCD_Init(void)
{
    BSP_LCD_Reset();

    /* Пока только аппаратный Reset.
       Инициализация ST7789 будет добавлена следующим этапом. */
}

/******************************************************************************
 * @brief Send Command
 ******************************************************************************/
void BSP_LCD_WriteCommand(uint8_t cmd)
{
    LCD_CS_Low();

    LCD_DC_Command();

    LCD_SendByte(cmd);

    LCD_CS_High();
}

/******************************************************************************
 * @brief Send Data
 ******************************************************************************/
void BSP_LCD_WriteData(uint8_t data)
{
    LCD_CS_Low();

    LCD_DC_Data();

    LCD_SendByte(data);

    LCD_CS_High();
}

/******************************************************************************
 * @brief Send Buffer
 ******************************************************************************/
void BSP_LCD_WriteBuffer(const uint8_t *buffer, uint16_t length)
{
    LCD_CS_Low();

    LCD_DC_Data();

    HAL_SPI_Transmit(&hspi2,
                     (uint8_t *)buffer,
                     length,
                     HAL_MAX_DELAY);

    LCD_CS_High();
}

/******************************************************************************
 * @brief Set Active Window
 ******************************************************************************/
void BSP_LCD_SetWindow(uint16_t x0,
                       uint16_t y0,
                       uint16_t x1,
                       uint16_t y1)
{
    /* Будет реализовано после запуска ST7789 */
}

/******************************************************************************
 * @brief Fill Screen
 ******************************************************************************/
void BSP_LCD_FillScreen(uint16_t color)
{
    /* Будет реализовано позже */
    (void)color;
}
