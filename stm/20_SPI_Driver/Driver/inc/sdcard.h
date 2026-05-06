#ifndef SDCARD_H_
#define SDCARD_H_

#include "stm32f407xx.h"
#include <stdint.h>

// Chip Select control (PB12)
#define SD_CS_LOW()   GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, RESET)
#define SD_CS_HIGH()  GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, SET)

// Public APIs
uint8_t SD_Init(void);
uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg);
uint8_t SD_WriteBlock(uint32_t block_addr, uint8_t *data);

// Low-level helpers
uint8_t SPI_TxRx(uint8_t data);
void SD_SendDummyClocks(uint8_t nbytes);

#endif
