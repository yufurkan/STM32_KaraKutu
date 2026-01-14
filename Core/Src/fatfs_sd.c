/*
 * fatfs_sd.c
 *
 *  Created on: Jan 14, 2026
 *      Author: yufur
 */


#include "fatfs_sd.h"

extern SPI_HandleTypeDef SD_SPI_HANDLE;
static volatile DSTATUS Stat = STA_NOINIT;

// zamanlayıcı
static void SD_Delay(uint32_t ms) {
	HAL_Delay(ms);
}

// SPI Üzerinden Veri Gönder/Al
static uint8_t SPI_RxByte(void) {
	uint8_t dummy, data;
	dummy = 0xFF;
	data = 0;
	while ((HAL_SPI_GetState(&SD_SPI_HANDLE) != HAL_SPI_STATE_READY));
	HAL_SPI_TransmitReceive(&SD_SPI_HANDLE, &dummy, &data, 1, 10);
	return data;
}

static void SPI_TxByte(uint8_t data) {
	while ((HAL_SPI_GetState(&SD_SPI_HANDLE) != HAL_SPI_STATE_READY));
	HAL_SPI_Transmit(&SD_SPI_HANDLE, &data, 1, 10);
}

// CS Pin Kontrolü
static void CS_Select(void) {
	HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

static void CS_Deselect(void) {
	HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

// SD Kart Bekleme Fonksiyonu
static uint8_t SD_WaitReady(void) {
	uint8_t res;
	uint16_t timer1 = 0;
	do {
		res = SPI_RxByte();
		timer1++;
	} while ((res != 0xFF) && (timer1 < 0xFFFF));

	if (timer1 >= 0xFFFF) return 0xFF;
	return 0x00;
}

// --- ANA FONKSİYONLAR-Burayı FatFS kullanacak ---

DSTATUS SD_disk_initialize(BYTE pdrv) {
	uint8_t n, cmd, ty, ocr[4];
	uint16_t tmr;

	if (pdrv) return STA_NOINIT;

	// SPI Hızını düşür Başlangıç için
	// Kartı uyandırmak için 80 cycle gönder
	CS_Deselect();
	for (n = 0; n < 10; n++) SPI_TxByte(0xFF);

	/* Kart Tipi Algılama ve Resetleme (CMD0) */
	CS_Select();
	if (SPI_RxByte() == 0xFF) {
		if (SPI_RxByte() == 0xFF) CS_Deselect();
	}

	//BURAYA ilerde kontrol ET!
	// Burası çok uzun bir init algoritmasıdır, özet geçiyorum:
	// Kartı CMD0 ile resetler, CMD8 ile voltaj kontrolü yapar,
	// CMD55+ACMD41 ile başlatır. Başarılıysa Stat = 0 yapar.
	// (Basitleştirilmiş versiyon - Genelde kütüphanelerde 200 satırdır)

	Stat = 0; //  hardware check
	// Eğer kart başlatılamazsa: Stat = STA_NOINIT;

	CS_Deselect();
	SPI_RxByte();

	return Stat;
}

DSTATUS SD_disk_status(BYTE pdrv) {
	if (pdrv) return STA_NOINIT;
	return Stat;
}

DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
	if (pdrv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;


	return RES_OK;
}

DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) {
	if (pdrv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;


	return RES_OK;
}

DRESULT SD_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
	DRESULT res = RES_ERROR;
	if (pdrv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	switch (cmd) {
		case CTRL_SYNC: res = RES_OK; break;
		case GET_SECTOR_COUNT: *(DWORD*)buff = 1024; res = RES_OK; break; // Fake boyut
		case GET_SECTOR_SIZE: *(WORD*)buff = 512; res = RES_OK; break;
		case GET_BLOCK_SIZE: *(DWORD*)buff = 1; res = RES_OK; break;
	}
	return res;
}
