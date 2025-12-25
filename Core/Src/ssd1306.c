/*
 * ssd1306.c
 *
 *  Created on: Dec 24, 2025
 *      Author: yufur
 */


#include "ssd1306.h"
#include <math.h>
#include <stdlib.h>

#include <string.h>

// SH1106 için 132 genişlik gerekiyor
static uint8_t SSD1306_Buffer[132 * 64 / 8];

static struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
} SSD1306;

void SSD1306_WriteCommand(uint8_t command) {
    HAL_I2C_Mem_Write(SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x00, 1, &command, 1, 10);
}

void SSD1306_Init(void) {
    HAL_Delay(100);

    SSD1306_WriteCommand(0xAE); // Ekran Kapalı
    SSD1306_WriteCommand(0xA1); // Segment Remap
    SSD1306_WriteCommand(0xC8); // Yön
    SSD1306_WriteCommand(0x81); // Kontrast
    SSD1306_WriteCommand(0xFF);
    SSD1306_WriteCommand(0xAF); // Ekran Açık

    SSD1306_Fill(Black);
    SSD1306_UpdateScreen();
}

void SSD1306_Fill(SSD1306_COLOR color) {
    uint32_t i;
    for(i = 0; i < sizeof(SSD1306_Buffer); i++) {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

void SSD1306_UpdateScreen(void) {
    for (uint8_t i = 0; i < 8; i++) {
        SSD1306_WriteCommand(0xB0 + i);
        // SH1106 OFSET AYARI (2 PİKSEL)
        // SH1106 hafızası daha falza olduğundan dolayı ekranda taşma riski var bu sebeple bu ayar yapılıyor.
        SSD1306_WriteCommand(0x02);
        SSD1306_WriteCommand(0x10);

        HAL_I2C_Mem_Write(SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x40, 1,
                          &SSD1306_Buffer[132 * i], 128, 100);
    }
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    if (color == White) {
        SSD1306_Buffer[x + (y / 8) * 132] |= (1 << (y % 8));
    } else {
        SSD1306_Buffer[x + (y / 8) * 132] &= ~(1 << (y % 8));
    }
}

void SSD1306_GotoXY(uint16_t x, uint16_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

void SSD1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color) {
    uint32_t i, b, j;

    // Font tablosunda yerini bul (ASCII)
    // boşluk 32 den başlar
    if (ch < 32 || ch > 126) return; // Geçersiz karakterse yazmasını engelle

    //
    for (i = 0; i < Font.FontHeight; i++) {
        b = Font.data[(ch - 32) * Font.FontHeight + i];
        for (j = 0; j < Font.FontWidth; j++) {
            if ((b << j) & 0x8000) {
                SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            } else {
                SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
            }
        }
    }

    // İmleci için sağa kaydır
    SSD1306.CurrentX += Font.FontWidth;
}


void SSD1306_Puts(char* str, FontDef Font, SSD1306_COLOR color) {
    while (*str) {
        SSD1306_WriteChar(*str, Font, color);
        str++;
    }
}
