/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <math.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t veritamponu[6];
int16_t AccX, AccY, AccZ; //gyro
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

#define MPU6050_WRITE_ADRES 0xD0 // (0x68 << 1)
#define PWR_MGMT_1_REG 0x6B //uyandırma bölgesi
#define ACCEL_XOUT_H_REG 0x3B // veri okumaya başlayadığımız nokta
#define MPU6050_ADDR 0x68



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

void MPU6050_Baslat(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  char mesaj[64];
    sprintf(mesaj, "I2C Tarama Basliyor\r\n");
    // HAL_UART_Transmit bilgisayara mesaj yolluyorum
    HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);

    int cihazSayisi = 0;

    // 0'dan 127'ye kadar tüm adresleri yokla
    for(uint16_t i = 0; i < 128; i++) {


        // Parametreler: (I2C Hattı, Adres, Deneme Sayısı, Bekleme Süresi)
        //&hi2c1 I2Cnın ayar stuctu
        if(HAL_I2C_IsDeviceReady(&hi2c1, (i << 1), 1, 10) == HAL_OK) {

            sprintf(mesaj, "BULUNDU! Adres: 0x%02X (Hex)\r\n", i);
            HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);
            cihazSayisi++;


            HAL_Delay(10);
        }
    }

    if (cihazSayisi == 0) {
        sprintf(mesaj, "Hicbir cihaz bulunamadi.\r\n");
        HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);
    } else {
        sprintf(mesaj, "--- Tarama Bitti. Toplam %d cihaz var ---\r\n", cihazSayisi);
        HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);
    }



    MPU6050_Baslat();


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */


	  HAL_StatusTypeDef okumaDurumu; //bu bir enumdur. HAL_OK    HAL_ERROR    HAL_BUSY   HAL_TIMEOUT maddelerini içeriyor. buradan mesaj kontrolünü sağlıyorum.
	  okumaDurumu = HAL_I2C_Mem_Read(&hi2c1,MPU6050_WRITE_ADRES, ACCEL_XOUT_H_REG, 1, veritamponu, 6, 100);//HAL_StatusTypeDef döndürür


	  //sensör jumper kabloların temassızlık yapması nedeniyle kopabiliyor. Koparsa 0x6B adresinin tekrar sıfırlanıp sensörün tekrar uyandırılması gerekiyor.
	  if(okumaDurumu==HAL_OK){
	  	 AccX= (int16_t)(veritamponu[0]<<8 | veritamponu[1]);
	     AccY= (int16_t)(veritamponu[2]<<8 | veritamponu[3]);
	     AccZ= (int16_t)(veritamponu[4]<<8 | veritamponu[5]);

	     //temassızlık durumunda veriler 0 oluyor sadece okumaDurumu==HAL_OK kontrolü yeterli gelmiyor sensör veri gönderdiğini söylüyor.
	     if(AccX == 0 && AccY == 0 && AccZ == 0)
	               {
	                   sprintf(mesaj, "!!! Sensör uyku modunda. Resetleniyor !!! \r\n");
	                   HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);

	                   MPU6050_Baslat();
	               }
	    	 sprintf(mesaj,
	    	         "AccX=%d  AccY=%d  AccZ=%d\r\n",
	    	         AccX, AccY, AccZ);

	     HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);
	     sprintf(mesaj, "  \r\n");
	     HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);

	     //tamsayı şeklinde bölmesin diye .0 koydum
	     float ax_g = AccX / 16384.0;
	     float ay_g = AccY / 16384.0;
	     float az_g = AccZ / 16384.0;

	     float toplamkuvvet = sqrt((ax_g * ax_g) + (ay_g * ay_g) + (az_g * az_g));

	     sprintf(mesaj, "Kuvvet: %.2f G \r\n", toplamkuvvet);
	     	  HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);

	     //2.5G kuvvetinden fazla değer ölçülürse kayıt işlenecek burada
	     //şimdilik kaza durumunu simüle ediyor
	     if (toplamkuvvet > 2.2)
	     	 {
	     	 sprintf(mesaj, "!!! KAZA ALGILANDI !!! \r\n");// etkinleştirmek için senöre parmağınızla vurun
	     	 HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);


	     	 HAL_Delay(1000);
	     	 }
	  }else{

		  sprintf(mesaj, "!!! Bağlantı koptu. Sensör Yeniden başlatılıyor !!! \r\n");
		  HAL_UART_Transmit(&huart2, (uint8_t*)mesaj, strlen(mesaj), 100);

		  MPU6050_Baslat();

	  }
	  HAL_Delay(500);

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE BEGIN 4 */


void MPU6050_Baslat(void)
{
    uint8_t uyandirici = 0;
    HAL_StatusTypeDef sonuc;

    //sensörün 0x6B registerini 0 yapıp uyandır
    sonuc = HAL_I2C_Mem_Write(&hi2c1, MPU6050_WRITE_ADRES, PWR_MGMT_1_REG, 1, &uyandirici, 1, 100);

    if(sonuc == HAL_OK)
    {
        char msg[] = "MPU6050 Baglantisi Kuruldu! \r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
    }
    else
    {

        char msg[] = "MPU6050 Cevap Vermiyor!\r\n";
        HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);

        //Peripheral Reset

        //Uzun süre sensörün voltaj kablosu çıkarılırsa tekrar sinyal alınamıyor ve program resetlenmek zorunda kalıyor
        //HAL_I2C_Mem_Writ işe yaramaz ise I2C1 modülünü resetler
        HAL_I2C_DeInit(&hi2c1);
        MX_I2C1_Init();
        HAL_Delay(100);
    }
}


/* USER CODE END 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
