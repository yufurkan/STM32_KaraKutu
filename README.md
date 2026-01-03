# 🚗 STM32 Black Box (Flight/Accident Recorder)

This project implements a robust **Black Box** system using the **STM32F446RE** microcontroller. It is designed to record multi-axis motion, environmental data, and critical events in real-time to an SD card, while visualizing status on an OLED screen.

## 🌟 Key Features
- **Multi-Sensor Fusion:** Reads data from 3 distinct sensors to capture comprehensive movement and environmental stats.
- **Real-time Visualization:** Displays G-force, altitude, and heading on a **0.96" I2C OLED Screen**.
- **Data Logging:** Records all sensor data and crash events to a **Micro SD Card** via **SPI** protocol for post-accident analysis.
- **Accident Detection:** Triggers an alarm and saves a "Crash Report" if G-force exceeds the critical threshold (e.g., 2.5G).
- **Fault Tolerance:** Auto-recovers from I2C bus failures (Zombie Sensor Mode) by resetting hardware peripherals.

## 🛠 Hardware Stack

### Microcontroller
- **MCU:** STM32 Nucleo-F446RE (ARM Cortex-M4)

### Sensors (GY-87 10-DOF Module)
1.  **MPU6050:** 6-Axis Accelerometer & Gyroscope (Motion & Impact)
2.  **HMC5883L:** 3-Axis Magnetometer (Compass/Heading)
3.  **BMP180:** Barometric Pressure Sensor (Altitude)

### Outputs & Storage
- **Display:** 0.96 inch OLED Display (SSD1306 Driver - I2C)
- **Storage:** Micro SD Card Module (SPI Interface)

## 🔌 Pin Configuration (Custom Setup)

| Component  | Pin Name | STM32 Pin | Protocol | Notes                     |
| :---       | :---     | :---      | :---     | :---                      |
| **GY-87** | SCL      | PB8 (D15) | I2C1     | Motion Sensors            |
|            | SDA      | PB9 (D14) | I2C1     |                           |
| **OLED** | SCL      | PB10      | I2C2     | Status Screen             |
|            | SDA      | PC12      | I2C2     |                           |
| **SD Card**| SCK      | **PB13** | **SPI2** | **Clock Signal** |
|            | MISO     | **PC2** | **SPI2** | **Master In Slave Out** |
|            | MOSI     | **PC1** | **SPI2** | **Master Out Slave In** |
|            | CS       | PB6 (D10) | GPIO     | Chip Select (User Config) |

## 🚀 How It Works
1. **System Check:** Initializes all sensors, OLED (I2C2), and mounts the SD Card (SPI2).
2. **Loop:**
   - Reads Acceleration (MPU6050) via I2C1.
   - Reads Compass Heading (HMC5883L).
   - Reads Altitude (BMP180).
   - Updates the OLED Display via I2C2.
   - Logs data string to `log.txt` on SD Card via SPI2.
3. **Safety:** Monitors connection health and performs hardware reset if sensors freeze.

---
*Developed by Yusuf Furkan Umutlu using STM32CubeIDE & HAL Library.*