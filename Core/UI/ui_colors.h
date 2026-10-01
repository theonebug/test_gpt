#ifndef UI_COLORS_H
#define UI_COLORS_H

#include "st7789.h"

/*----------------------------------------------------------
    Background
----------------------------------------------------------*/

#define UI_COLOR_BACKGROUND      0x0016

/*----------------------------------------------------------
    Panels
----------------------------------------------------------*/

#define UI_COLOR_PANEL           0x10A2
#define UI_COLOR_BORDER          0x4B55

/*----------------------------------------------------------
    Text
----------------------------------------------------------*/

#define UI_COLOR_TITLE           ST7789_YELLOW
#define UI_COLOR_LABEL           ST7789_YELLOW
#define UI_COLOR_VALUE           ST7789_CYAN

/*----------------------------------------------------------
    Status
----------------------------------------------------------*/

#define UI_COLOR_OK              ST7789_GREEN
#define UI_COLOR_WARNING         ST7789_YELLOW
#define UI_COLOR_ERROR           ST7789_RED

/*----------------------------------------------------------
    Icons
----------------------------------------------------------*/

#define UI_COLOR_ICON_OK         0x4FEC   /* светло-зелёный */
#define UI_COLOR_ICON_ERROR      0xF9A6   /* ярко-красный   */
#define UI_COLOR_ICON_GLYPH      ST7789_WHITE

/*----------------------------------------------------------
    Big digits
----------------------------------------------------------*/

#define UI_COLOR_DIGITS          0xFFE0

#endif
