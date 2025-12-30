/*
 * i2c_scanner.h
 *
 *  Created on: Dec 30, 2025
 *      Author: yufur
 */

#ifndef INC_I2C_SCANNER_H_
#define INC_I2C_SCANNER_H_

#include "stm32f4xx_hal.h"


// I2C hattını parametre olarak al -- Uart da uyarlanabilir
void I2C_Scanner_Baslat(I2C_HandleTypeDef *hi2c, UART_HandleTypeDef *huart, char *HatIsmi);

#endif /* INC_I2C_SCANNER_H_ */
