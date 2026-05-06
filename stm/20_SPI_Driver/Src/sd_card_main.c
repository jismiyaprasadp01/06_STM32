#include "stm32f407xx.h"
#include "stm32f407xx_gpio._driver.h"
#include "stm32f407xx_spi_driver.h"
#include "sdcard.h"
#include <string.h>
#include <stdio.h>

// ================= GLOBAL =================
SPI_Handle_t SPI2handle;

// ================= DELAY =================
void delay(void)
{
    for(uint32_t i = 0; i < 500000; i++);
}

// ================= SPI GPIO =================
void SPI2_GPIOInits(void)
{
    GPIO_Handle_t SPIPins, NSSpin;

    SPIPins.pGPIOx = GPIOB;
    SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
    SPIPins.GPIO_PinConfig.GPIO_PinOPtype = GPIO_OP_TYPE_PP;
    SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    // SCLK
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&SPIPins);

    // MOSI
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&SPIPins);

    // MISO
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
    SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    GPIO_Init(&SPIPins);

    // CS (PB12)
    NSSpin.pGPIOx = GPIOB;
    NSSpin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    NSSpin.GPIO_PinConfig.GPIO_PinOPtype = GPIO_OP_TYPE_PP;
    NSSpin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    NSSpin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    NSSpin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    GPIO_Init(&NSSpin);

    GPIO_WriteToOutputPin(GPIOB, GPIO_PIN_NO_12, SET);
}

// ================= SPI INIT =================
void SPI2_Init(void)
{
    SPI2handle.pSPIx = SPI2;

    SPI2handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
    SPI2handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
    SPI2handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV32;
    SPI2handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
    SPI2handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
    SPI2handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
    SPI2handle.SPIConfig.SPI_SSM = SPI_SSM_EN;

    SPI_Init(&SPI2handle);
}

// ================= BUTTON =================
void GPIO_BtnInit(void)
{
    GPIO_Handle_t GPIOBtn;

    GPIOBtn.pGPIOx = GPIOA;
    GPIOBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    GPIOBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IN;
    GPIOBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    GPIOBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;

    GPIO_Init(&GPIOBtn);
}
void LED_Init(void)
{
    GPIO_Handle_t led;

    led.pGPIOx = GPIOD;
    led.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    led.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    led.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
    led.GPIO_PinConfig.GPIO_PinOPtype = GPIO_OP_TYPE_PP;
    led.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

    GPIO_Init(&led);
}
// ================= ULTRASONIC =================
// 👉 Replace with your previous working function
uint32_t get_distance(void)
{
    return 25; // dummy value for testing
}

// ================= MAIN =================

int main(void)
{
	LED_Init();
    uint8_t buffer[512];

    GPIO_BtnInit();
    SPI2_GPIOInits();
    SPI2_Init();

    SPI_SSIConfig(SPI2, ENABLE);
    SPI_SSOEConfig(SPI2, DISABLE);
    SPI_PeripheralControl(SPI2, ENABLE);

    delay();

    // Initialize SD card
    if(SD_Init() != 0)
    {
        while(1); // SD init failed
    }

    uint32_t block = 0;

    while(1)
    {
        if(GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0))
        {
        	GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_12);
            delay(); // debounce

           // uint32_t dist = get_distance();

            // Clear buffer
            memset(buffer, 0, 512);
            strcpy((char*)buffer, "HELLO SD CARD\r\n");

            // Write text into buffer
           // sprintf((char*)buffer, "Distance: %lu cm\r\n", dist);

            // Write to SD card
            SD_WriteBlock(block++, buffer);

            // wait until button released
            while(GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0));
        }
    }
}
