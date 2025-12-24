/*
 * ssd1306.h
 *
 *  Created on: Dec 24, 2025
 *      Author: yufur
 */

#ifndef SSD1306_H
#define SSD1306_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// AYARLAR
#define SSD1306_I2C_PORT        &hi2c2
#define SSD1306_I2C_ADDR        0x78

// Ekran Boyutları
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

// Renkler
typedef enum {
    Black = 0x00,
    White = 0x01
} SSD1306_COLOR;


typedef struct {
    const uint8_t FontWidth;    /*!< Font width in pixels */
    uint8_t FontHeight;   /*!< Font height in pixels */
    const uint16_t *data; /*!< Pointer to data font data array */
} FontDef;

// FONKSİYONLAR
void SSD1306_Init(void);
void SSD1306_Fill(SSD1306_COLOR color);
void SSD1306_UpdateScreen(void);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
void SSD1306_GotoXY(uint16_t x, uint16_t y);
void SSD1306_Puts(char* str, FontDef Font, SSD1306_COLOR color);

extern I2C_HandleTypeDef hi2c2;

#endif
