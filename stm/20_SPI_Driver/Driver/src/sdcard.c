#include "sdcard.h"
#include "stm32f407xx_spi_driver.h"

// ===== Low-level SPI byte =====
uint8_t SPI_TxRx(uint8_t data)
{
    uint8_t rx;
    SPI_TransmitReceive(SPI2, &data, &rx, 1);
    return rx;
}

// ===== Send dummy clocks =====
void SD_SendDummyClocks(uint8_t nbytes)
{
    for(uint8_t i = 0; i < nbytes; i++)
        SPI_TxRx(0xFF);
}

// ===== Send SD command =====
uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg)
{
    uint8_t response;
    uint8_t crc = 0x01;

    // Proper CRC only needed for CMD0/CMD8 in SPI mode
    if(cmd == 0)  crc = 0x95; // CMD0
    if(cmd == 8)  crc = 0x87; // CMD8

    SD_CS_LOW();

    // Command packet: [0x40|cmd][arg(4)][crc]
    SPI_TxRx(0x40 | cmd);
    SPI_TxRx((arg >> 24) & 0xFF);
    SPI_TxRx((arg >> 16) & 0xFF);
    SPI_TxRx((arg >> 8)  & 0xFF);
    SPI_TxRx((arg)       & 0xFF);
    SPI_TxRx(crc);

    // Wait for a response (MSB=0)
    for(int i = 0; i < 10; i++)
    {
        response = SPI_TxRx(0xFF);
        if(!(response & 0x80))
            break;
    }

    return response;
}

// ===== Initialize SD card (SPI mode) =====
uint8_t SD_Init(void)
{
    uint8_t resp;

    SD_CS_HIGH();

    // 80+ clock cycles with CS high
    SD_SendDummyClocks(10);

    // CMD0: go idle
    resp = SD_SendCommand(0, 0);
    SD_CS_HIGH(); SPI_TxRx(0xFF);
    if(resp != 0x01) return 1;

    // CMD8: check voltage range
    resp = SD_SendCommand(8, 0x000001AA);
    SD_CS_HIGH(); SPI_TxRx(0xFF);
    // Some older cards may not support CMD8; continue anyway

    // ACMD41 loop (CMD55 + CMD41)
    do
    {
        resp = SD_SendCommand(55, 0);
        SD_CS_HIGH(); SPI_TxRx(0xFF);

        resp = SD_SendCommand(41, 0x40000000);
        SD_CS_HIGH(); SPI_TxRx(0xFF);

    } while(resp != 0x00); // ready

    return 0; // success
}

// ===== Write one 512-byte block =====
uint8_t SD_WriteBlock(uint32_t block_addr, uint8_t *data)
{
    uint8_t resp;

    // For SDSC cards, use byte address: block_addr *= 512;
    // For SDHC/SDXC, block address is already in sectors.
    // Here we assume SDHC (most common). Adjust if needed.

    resp = SD_SendCommand(24, block_addr); // CMD24
    if(resp != 0x00)
    {
        SD_CS_HIGH(); SPI_TxRx(0xFF);
        return 1;
    }

    // Start token
    SPI_TxRx(0xFE);

    // Send 512 bytes
    for(int i = 0; i < 512; i++)
        SPI_TxRx(data[i]);

    // Dummy CRC
    SPI_TxRx(0xFF);
    SPI_TxRx(0xFF);

    // Data response
    resp = SPI_TxRx(0xFF);
    if((resp & 0x1F) != 0x05)
    {
        SD_CS_HIGH(); SPI_TxRx(0xFF);
        return 2;
    }

    // Wait until card is not busy
    while(SPI_TxRx(0xFF) == 0x00);

    SD_CS_HIGH();
    SPI_TxRx(0xFF);

    return 0;
}
