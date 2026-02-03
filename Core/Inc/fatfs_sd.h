/*
 * fatfs_sd.h
 *
 *  Created on: Jan 14, 2026
 *      Author: yufur
 */

#ifndef __FATFS_SD_H
#define __FATFS_SD_H

#include "main.h"
#include "diskio.h"

#define SD_SPI_HANDLE hspi2  // SPI2


#define SD_CS_GPIO_Port GPIOB
#define SD_CS_Pin GPIO_PIN_6

DSTATUS SD_disk_initialize (BYTE pdrv);
DSTATUS SD_disk_status (BYTE pdrv);
DRESULT SD_disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
DRESULT SD_disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);

#endif /* INC_FATFS_SD_H_ */
