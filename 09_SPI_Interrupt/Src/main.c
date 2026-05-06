#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "uart.h"
#include "adc.h"

// spi1 == apb1 AF5

SPI_HandleTypeDef hspi1;

void spi1_init(void);

uint8_t tx_buffer[10] = {10,20,30,40,50,60,70,80,90,100};
uint8_t rx_buffer[10];

int counter;

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi1)
{
	// do something
	counter++;
}

int main(void)
{
	HAL_Init();
	spi1_init();

	HAL_SPI_TransmitReceive_IT(&hspi1, tx_buffer, rx_buffer, 10);

	while(1)
	{

	}
}

/* PA5 ---- SPI1_SCK
PA6 ---- SPI1_MISO
PA7 ---- SPI1_MOSI */

void spi1_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_SPI1_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// configure SPI module

	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler =SPI_BAUDRATEPRESCALER_8;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

	HAL_SPI_Init(&hspi1);

	HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(SPI1_IRQn);
}

void SPI1_IRQHandler(void)
{
	HAL_SPI_IRQHandler(&hspi1);
}

void SysTick_Handler(void)
{
	HAL_IncTick();
}


