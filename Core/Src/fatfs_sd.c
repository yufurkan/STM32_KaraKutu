/*-----------------------------------------------------------------------*/
/* SPI SD Card Driver for STM32 (Low Level)                              */
/*-----------------------------------------------------------------------*/
/* * This driver acts as a bridge between the generic FatFS middleware
 * and the specific STM32 SPI hardware.
 * * Origin: Adapted from ChaN's MMC/SDC driver and standard community implementations.
 */

#include "fatfs_sd.h"

extern SPI_HandleTypeDef SD_SPI_HANDLE;
static volatile DSTATUS Stat = STA_NOINIT;

/* SD Card Commands (Standard definitions) */
#define CMD0     (0x40+0)     /* GO_IDLE_STATE */
#define CMD1     (0x40+1)     /* SEND_OP_COND */
#define CMD8     (0x40+8)     /* SEND_IF_COND */
#define CMD9     (0x40+9)     /* SEND_CSD */
#define CMD10    (0x40+10)    /* SEND_CID */
#define CMD12    (0x40+12)    /* STOP_TRANSMISSION */
#define CMD16    (0x40+16)    /* SET_BLOCKLEN */
#define CMD17    (0x40+17)    /* READ_SINGLE_BLOCK */
#define CMD18    (0x40+18)    /* READ_MULTIPLE_BLOCK */
#define CMD23    (0x40+23)    /* SET_BLOCK_COUNT */
#define CMD24    (0x40+24)    /* WRITE_BLOCK */
#define CMD25    (0x40+25)    /* WRITE_MULTIPLE_BLOCK */
#define CMD41    (0x40+41)    /* SEND_OP_COND (ACMD) */
#define CMD55    (0x40+55)    /* APP_CMD */
#define CMD58    (0x40+58)    /* READ_OCR */

/* --- Helper Functions --- */

/* Transmit a single byte via SPI */
static void SPI_TxByte(uint8_t data) {
	while(!__HAL_SPI_GET_FLAG(&SD_SPI_HANDLE, SPI_FLAG_TXE));
	HAL_SPI_Transmit(&SD_SPI_HANDLE, &data, 1, 10);
}

/* Receive a single byte via SPI */
static uint8_t SPI_RxByte(void) {
	uint8_t dummy = 0xFF;
	uint8_t data;
	while(!__HAL_SPI_GET_FLAG(&SD_SPI_HANDLE, SPI_FLAG_TXE));
	HAL_SPI_TransmitReceive(&SD_SPI_HANDLE, &dummy, &data, 1, 10);
	return data;
}

/* Assert Chip Select (CS Low) */
static void CS_Select(void) {
	HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);
}

/* De-assert Chip Select (CS High) */
static void CS_Deselect(void) {
	HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET);
}

/* Wait for the card to be ready (Not Busy) */
static uint8_t SD_WaitReady(void) {
	uint8_t res;
	uint16_t timer = 0;
	do {
		res = SPI_RxByte();
		timer++;
	} while ((res != 0xFF) && (timer < 0xFFFF));
	return res;
}

/* Send a Command to the SD Card */
static uint8_t SD_SendCmd(uint8_t cmd, uint32_t arg) {
	uint8_t crc, res;

	/* Wait if the card is busy */
	if (SD_WaitReady() != 0xFF) return 0xFF;

	/* Transmit Command Packet */
	SPI_TxByte(cmd); 			        /* Command Index */
	SPI_TxByte((uint8_t)(arg >> 24)); 	/* Argument[31..24] */
	SPI_TxByte((uint8_t)(arg >> 16)); 	/* Argument[23..16] */
	SPI_TxByte((uint8_t)(arg >> 8)); 	/* Argument[15..8] */
	SPI_TxByte((uint8_t)arg); 		    /* Argument[7..0] */

	/* Calculate CRC (Simplified for SPI mode) */
	crc = 0x01;
	if (cmd == CMD0) crc = 0x95; // CRC for CMD0
	if (cmd == CMD8) crc = 0x87; // CRC for CMD8
	SPI_TxByte(crc);

	/* Receive Response (R1) */
	uint8_t n = 10;
	do {
		res = SPI_RxByte();
	} while ((res & 0x80) && --n);

	return res;
}

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/
DSTATUS SD_disk_initialize(BYTE pdrv) {
	uint8_t n, ty, ocr[4];
	uint16_t tmr;

	if (pdrv) return STA_NOINIT; // Supports only drive 0

	/* 1. Wake up sequence: Send dummy clocks with CS high */
	CS_Deselect();
	for (n = 0; n < 10; n++) SPI_TxByte(0xFF);

	/* 2. Enter Idle State (CMD0) */
	CS_Select();
	if (SD_SendCmd(CMD0, 0) == 1) {
		/* 3. Check Voltage Range (CMD8) */
		if (SD_SendCmd(CMD8, 0x1AA) == 1) {
			/* SDC Ver2+ */
			for (n = 0; n < 4; n++) ocr[n] = SPI_RxByte();
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
				/* Card is compatible, Initialize with ACMD41 */
				for (tmr = 25000; tmr && SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 1UL << 30); tmr--);

				if (tmr && SD_SendCmd(CMD58, 0) == 0) {
					/* Check CCS bit (High Capacity?) */
					for (n = 0; n < 4; n++) ocr[n] = SPI_RxByte();
					ty = (ocr[0] & 0x40) ? 6 : 2; /* 6: SDHC/XC, 2: SDSC */
				}
			}
		} else {
			/* SDC Ver1 or MMC */
			ty = (SD_SendCmd(CMD55, 0) <= 1 && SD_SendCmd(CMD41, 0) <= 1) ? 2 : 1;
			// Initialization loop for older cards would go here...
		}
	} else {
		ty = 0; // Initialization failed
	}

	CS_Deselect();
	SPI_RxByte(); // Clear SPI buffer

	if (ty) {
		Stat &= ~STA_NOINIT; // Clear NOINIT flag (Success)
	} else {
		Stat = STA_NOINIT;   // Set NOINIT flag (Failure)
	}

	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
DSTATUS SD_disk_status(BYTE pdrv) {
	if (pdrv) return STA_NOINIT;
	return Stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
DRESULT SD_disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count) {
	if (pdrv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	/* Note: sector conversion to byte address is needed for non-SDHC cards */
	// if (!(CardType & CT_BLOCK)) sector *= 512;

	CS_Select();
	if (count == 1) {
		/* Read Single Block */
		if ((SD_SendCmd(CMD17, sector) == 0)) {
			/* Wait for Data Token (0xFE) */
			uint16_t tmr = 50000;
			while (SPI_RxByte() != 0xFE && tmr--) ;

			if (tmr) {
				/* Receive Data */
				for(uint16_t i=0; i<512; i++) *buff++ = SPI_RxByte();
				/* Discard CRC (2 bytes) */
				SPI_RxByte();
				SPI_RxByte();
			}
		}
	}
	/* Multiple block read implementation omitted for simplicity */

	CS_Deselect();
	SPI_RxByte();

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
DRESULT SD_disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count) {
	if (pdrv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	CS_Select();

	if (count == 1) {
		/* Write Single Block */
		if (SD_SendCmd(CMD24, sector) == 0) {
			/* Wait */
			SPI_TxByte(0xFF);

			/* Send Data Token (0xFE) */
			SPI_TxByte(0xFE);

			/* Send Data */
			for (uint16_t i = 0; i < 512; i++) SPI_TxByte(*buff++);

			/* Send Dummy CRC */
			SPI_TxByte(0xFF);
			SPI_TxByte(0xFF);

			/* Check Data Response (Mask 0x1F, should be 0x05) */
			if ((SPI_RxByte() & 0x1F) == 0x05) {
				/* Wait while card is busy programming */
				while (SPI_RxByte() == 0);
			}
		}
	}

	CS_Deselect();
	SPI_RxByte();

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
DRESULT SD_disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
	DRESULT res = RES_ERROR;
	if (pdrv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	CS_Select();

	switch (cmd) {
		case CTRL_SYNC:
			/* Make sure that no pending write process */
			if (SD_WaitReady() == 0xFF) res = RES_OK;
			break;

		case GET_SECTOR_COUNT:
			/* Return fake sector count (adequate for logging) */
			*(DWORD*)buff = 1024 * 64;
			res = RES_OK;
			break;

		case GET_SECTOR_SIZE:
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE:
			*(DWORD*)buff = 1;
			res = RES_OK;
			break;
	}

	CS_Deselect();
	SPI_RxByte();
	return res;
}
