#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "uart.h"
#include "adc.h"

#define DEVID_R       0x00
#define POWER_CTL     0x2D
#define DATA_FORMAT   0x31
#define DEVICE_ADDR   0x53<<1
#define FOUR_G        0x01
#define RESET         0x00
#define SET_MEASURE_B 0x08
#define DATA_START_ADDR 0x32
#define FOUR_G_SCLAE_FACT 0.0078;

I2C_HandleTypeDef hi2c1;
void i2c1_init(void);
void adx_write(uint8_t reg, uint8_t value);
void adx_read_values(uint8_t reg);
void adx_read_address(uint8_t reg);
void adx_init(void);

uint8_t data_rec[6];
uint8_t device_id;

int16_t x,y,z;
float xg,yg,zg;

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef * hi2c)
{
	adx_read_values(DATA_START_ADDR);

	x = ((data_rec[1] << 8) | data_rec[0]);
	y = ((data_rec[3] << 8) | data_rec[2]);
	z = ((data_rec[5] << 8) | data_rec[4]);

	xg = x * FOUR_G_SCLAE_FACT;
	yg = y * FOUR_G_SCLAE_FACT;
	zg = z * FOUR_G_SCLAE_FACT;

	HAL_Delay(10);
}

int main(void)
{
	HAL_Init();
	i2c1_init();
	adx_init();

 while(1)
 {

 }
}

void adx_write(uint8_t reg, uint8_t value)
{
	uint8_t data[2];
	data[0] = reg;
	data[1] = value;

	HAL_I2C_Master_Transmit(&hi2c1,DEVICE_ADDR,data,2,100);
}

void adx_read_values(uint8_t reg)
{
	HAL_I2C_Mem_Read(&hi2c1, DEVICE_ADDR, reg, 1, (uint8_t*)data_rec,6,100);
}

void adx_read_address(uint8_t reg)
{
	HAL_I2C_Mem_Read(&hi2c1, DEVICE_ADDR, reg, 1,&device_id,1,100);
}

void adx_init(void)
{
	//1. Read device ID
	adx_read_address(DEVID_R);

	//2. Set data format +/- 4g
	adx_write(DATA_FORMAT, FOUR_G);

	//3. Reset all bits
	adx_write(POWER_CTL, RESET);

	//4. Config PWR control
	adx_write(POWER_CTL, SET_MEASURE_B);
}

void i2c1_init(void)
{
	/* I2C1_SCL --- PB6
	   I2C1_SDA --- PB7 */

	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;

	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// configure i2c

	__HAL_RCC_I2C1_CLK_ENABLE();
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 400000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

	HAL_I2C_Init(&hi2c1);

	HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
}

void I2C1_EV_IRQHandler(void)
{
	HAL_I2C_EV_IRQHandler(&hi2c1);
}

void SysTick_Handler(void)
{
	HAL_IncTick();
}


