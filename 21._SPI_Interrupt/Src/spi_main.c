#include "stm32f407xx.h"
#include <string.h>

SPI_Handle_t SPI2Handle;

volatile uint8_t buttonPressed = 0;
volatile uint8_t transferComplete = 0;

uint8_t txData = 'H';
uint8_t rxData;

/*
PB13 -> SCK
PB15 -> MOSI
PB14 -> MISO
PB12 -> NSS
PA0  -> Button
*/

void delay(void)
{
    for(uint32_t i=0;i<500000;i++);
}

/* ================= BUTTON INIT ================= */

void Button_Init(void)
{
    GPIO_Handle_t Button;

    Button.pGPIOx = GPIOA;
    Button.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
    Button.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
    Button.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    Button.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    GPIO_Init(&Button);

    GPIO_IRQPriorityConfig(IRQ_NO_EXTI0,NVIC_IRQ_PR15);
    GPIO_IRQInterruptConfig(IRQ_NO_EXTI0,ENABLE);
}

/* ================= SPI GPIO ================= */

void SPI2_GPIOInit(void)
{
    GPIO_Handle_t SPIPins;
    GPIO_Handle_t NSSPin;

    SPIPins.pGPIOx = GPIOB;
    SPIPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    SPIPins.GPIO_PinConfig.GPIO_PinAltFunMode = 5;
    SPIPins.GPIO_PinConfig.GPIO_PinOPtype = GPIO_OP_TYPE_PP;
    SPIPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    SPIPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    // SCK
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
    GPIO_Init(&SPIPins);

    // MISO
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
    GPIO_Init(&SPIPins);

    // MOSI
    SPIPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
    GPIO_Init(&SPIPins);

    // NSS manual GPIO
    NSSPin.pGPIOx = GPIOB;
    NSSPin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
    NSSPin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
    NSSPin.GPIO_PinConfig.GPIO_PinOPtype = GPIO_OP_TYPE_PP;
    NSSPin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
    NSSPin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    GPIO_Init(&NSSPin);

    GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_12,SET);
}

/* ================= SPI INIT ================= */

void SPI2_Init(void)
{
    SPI2Handle.pSPIx = SPI2;

    SPI2Handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;
    SPI2Handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
    SPI2Handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV64;
    SPI2Handle.SPIConfig.SPI_DFF = SPI_DFF_8BITS;
    SPI2Handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
    SPI2Handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
    SPI2Handle.SPIConfig.SPI_SSM = SPI_SSM_EN;

    SPI_Init(&SPI2Handle);
}

/* ================= MAIN ================= */

int main(void)
{
    Button_Init();
    SPI2_GPIOInit();
    SPI2_Init();

    SPI_SSIConfig(SPI2,ENABLE);
    SPI_PeripheralControl(SPI2,ENABLE);

    SPI_IRQInterruptConfig(IRQ_NO_SPI2,ENABLE);
    SPI_IRQPriorityConfig(IRQ_NO_SPI2,NVIC_IRQ_PR15);

    while(1)
    {
        if(buttonPressed)
        {
            delay();

            transferComplete = 0;

            // NSS LOW
            GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_12,RESET);

            SPI_SendDataReceiveIT(&SPI2Handle,
                                  &txData,
                                  &rxData,
                                  1);

            while(transferComplete == 0);

            // NSS HIGH
            GPIO_WriteToOutputPin(GPIOB,GPIO_PIN_NO_12,SET);

            buttonPressed = 0;
        }
    }
}

/* ================= BUTTON ISR ================= */

void EXTI0_IRQHandler(void)
{
    GPIO_IRQHandling(GPIO_PIN_NO_0);

    buttonPressed = 1;
}

/* ================= SPI ISR ================= */

void SPI2_IRQHandler(void)
{
    SPI_IRQHandling(&SPI2Handle);
}

/* ================= CALLBACK ================= */

void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,uint8_t AppEv)
{
    if(AppEv == SPI_EVENT_RX_CMPLT)
    {
        transferComplete = 1;

        // received byte available in rxData
    }
}
