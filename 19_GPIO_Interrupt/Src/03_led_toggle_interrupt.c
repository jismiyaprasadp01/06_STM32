#include "stm32f407xx.h"
//#include "stm32f407xx_gpio._driver.h"
#include <stdint.h>
#include "string.h"
#define HIGH 1
#define LOW 0
#define BTN_PRESSED LOW

void delay(void)
{
	//this will introduce ~200ms delay when system clock is 16MZ
	for(uint32_t i=0; i<500000/2;i++);
}


int main()
{
	GPIO_Handle_t GpioLed, GpioBtn;

	memset(&GpioLed,0,sizeof(GpioLed));
	memset(&GpioBtn,0,sizeof(GpioBtn));

	// LED config
	GpioLed.pGPIOx = GPIOD;
	GpioLed.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GpioLed.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GpioLed.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioLed.GPIO_PinConfig.GPIO_PinOPtype = GPIO_OP_TYPE_PP;
	GpioLed.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOD, ENABLE);
	GPIO_Init(&GpioLed);

	// Button config
	GpioBtn.pGPIOx = GPIOA;
	GpioBtn.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_0;
	GpioBtn.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_IT_FT;
	GpioBtn.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GpioBtn.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;

	GPIO_PeriClockControl(GPIOA, ENABLE);
	GPIO_Init(&GpioBtn);

	// IRQ config (FIXED)
	GPIO_IRQPriorityConfig(IRQ_NO_EXTI0, NVIC_IRQ_PR0);
	GPIO_IRQInterruptConfig(IRQ_NO_EXTI0, ENABLE);

	while(1);
}


// Correct ISR
//void EXTI0_IRQHandler(void)
//{
////	GPIO_IRQHandling(GPIO_PIN_NO_0);
////
////	// simple debounce check
////	if(GPIO_ReadFromInputPin(GPIOA, GPIO_PIN_NO_0) == 0)
////	{
////		GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_13);
////	}
//	//while(1);
//
//}
void EXTI0_IRQHandler(void)
{
	GPIO_IRQHandling(GPIO_PIN_NO_0);
	GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_13);
}
