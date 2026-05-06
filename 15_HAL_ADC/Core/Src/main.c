#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "uart.h"
#include "adc.h"


uint32_t sensor_value;
int main(void)
{
	HAL_Init();
	uart_init();
	adc_init_start();

 while(1)
 {
	 sensor_value = pa0_adc_read();
	//printf("Printf is being used! \n\r");
	 //HAL_Delay(10);
 }

}

void SysTick_Handler(void)
{
	HAL_IncTick();
}

