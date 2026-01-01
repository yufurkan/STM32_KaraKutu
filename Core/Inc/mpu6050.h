/*
 * mpu6050.h
 *
 *  Created on: Dec 30, 2025
 *      Author: yufur
 */


#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "stm32f4xx_hal.h"

// define'ları buraya taşıdım
#define MPU6050_ADDR         0xD0 // (0x68 << 1)
#define PWR_MGMT_1_REG       0x6B // Uyandırma bölgesi
#define ACCEL_XOUT_H_REG     0x3B // Veri okumaya başladığımız nokta


// Artık AccX, AccY tek tek gezmeyecek, bu kutunun içinde duracak
typedef struct {
    int16_t AccX;
    int16_t AccY;
    int16_t AccZ;
    float ToplamKuvvet; // G kuvveti hesabını kütüphane yapacak
} MPU6050_t;

// --- Fonksiyonlar ---
uint8_t MPU6050_Baslat(I2C_HandleTypeDef *hi2c);
void MPU6050_Oku(I2C_HandleTypeDef *hi2c, MPU6050_t *SensorVerisi);

#endif /* INC_MPU6050_H_ */
