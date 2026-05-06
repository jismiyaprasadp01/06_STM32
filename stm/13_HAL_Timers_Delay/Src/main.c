#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "uart.h"
#include "adc.h"
#include "tim.h"


int main(void)
{
	HAL_Init();
	tim_timebase_init();
 while(1)
 {

 }

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	// do something
	printf("A second just elapsed!\n\r");
}

void SysTick_Handler(void)
{
	HAL_IncTick();
}




