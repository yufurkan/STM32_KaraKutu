/*
 * mpu6050.c
 *
 *  Created on: Dec 30, 2025
 *      Author: yufur
 */


#include "mpu6050.h"
#include <math.h>

// Sensörü Uyandırma Fonksiyonu sonucu döndürüyor
uint8_t MPU6050_Baslat(I2C_HandleTypeDef *hi2c)
{
    uint8_t uyandirici = 0;
    HAL_StatusTypeDef sonuc;

    // Sensörün 0x6B registerini 0 yap- uyandır
    sonuc = HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &uyandirici, 1, 100);

    if(sonuc == HAL_OK)
    {
        return 1;
    }
    return 0;
}

// Sensör Okuma ve Hesaplama Fonksiyonu
void MPU6050_Oku(I2C_HandleTypeDef *hi2c, MPU6050_t *SensorVerisi)
{
    uint8_t veritamponu[6]; // Artık sadece burada lazım
    HAL_StatusTypeDef okumaDurumu;

    // Verileri Oku
    okumaDurumu = HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, veritamponu, 6, 100);

    if(okumaDurumu == HAL_OK)
    {
        // Byte birleştirme işlemleri
        SensorVerisi->AccX = (int16_t)(veritamponu[0] << 8 | veritamponu[1]);
        SensorVerisi->AccY = (int16_t)(veritamponu[2] << 8 | veritamponu[3]);
        SensorVerisi->AccZ = (int16_t)(veritamponu[4] << 8 | veritamponu[5]);

        // G juvvetine çevir
        float ax_g = SensorVerisi->AccX / 16384.0;
        float ay_g = SensorVerisi->AccY / 16384.0;
        float az_g = SensorVerisi->AccZ / 16384.0;

        SensorVerisi->ToplamKuvvet = sqrt((ax_g * ax_g) + (ay_g * ay_g) + (az_g * az_g));
    }
    else
    {
        // Okuma hatası durumunda veriler sıfırlanıyor. İleride sıfır olan veriler için sensör yeniden başlatılıyor
        SensorVerisi->AccX = 0;
        SensorVerisi->AccY = 0;
        SensorVerisi->AccZ = 0;
    }
}
