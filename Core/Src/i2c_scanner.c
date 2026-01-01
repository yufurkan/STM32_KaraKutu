/*
 * i2c_scanner.c
 *
 *  Created on: Dec 30, 2025
 *      Author: yufur
 */


#include "i2c_scanner.h"
#include <stdio.h>
#include <string.h>

void I2C_Scanner_Baslat(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, char *HatIsmi)
{
    char mesaj[64];
    int cihazSayisi = 0;

    sprintf(mesaj, "--- %s Taramasi Basliyor ---\r\n", HatIsmi);
    HAL_UART_Transmit(huart, (uint8_t*)mesaj, strlen(mesaj), 100);

    // 0'dan 127'ye kadar tüm adresleri yokla
    for(uint16_t i = 0; i < 128; i++)
    {
        // I2C Hattı, Adres, Deneme Sayısı, Bekleme Süresi
        // Adresi sola 1 kaydırıyoruz (i << 1) son bit R/W bitidir.
        if(HAL_I2C_IsDeviceReady(hi2c, (i << 1), 1, 10) == HAL_OK)
        {
            sprintf(mesaj, "[%s] BULUNDU! Adres: 0x%02X (Hex)\r\n", HatIsmi, i);
            HAL_UART_Transmit(huart, (uint8_t*)mesaj, strlen(mesaj), 100);
            cihazSayisi++;
            HAL_Delay(5);
        }
    }


    if (cihazSayisi == 0) {
        sprintf(mesaj, "[%s] Hicbir cihaz bulunamadi.\r\n\n", HatIsmi);
    } else {
        sprintf(mesaj, "[%s] Tarama Bitti. Toplam %d cihaz var.\r\n\n", HatIsmi, cihazSayisi);
    }
    HAL_UART_Transmit(huart, (uint8_t*)mesaj, strlen(mesaj), 100);
}
