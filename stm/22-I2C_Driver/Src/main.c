#include "stm32f407xx.h"
#include "stm32f407xx_i2c_driver.h"
#include "stm32f407xx_gpio._driver.h"
#include <string.h>

I2C_Handle_t I2C1Handle;

void delay(void)
{
    for(uint32_t i=0;i<500000;i++);
}

/*
* PB6 -> I2C1_SCL
* PB7 -> I2C1_SDA
*/
void I2C1_GPIOInits(void)
{
    GPIO_Handle_t I2CPins;

    I2CPins.pGPIOx = GPIOB;
    I2CPins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    I2CPins.GPIO_PinConfig.GPIO_PinAltFunMode = 4; // AF4 = I2C
    I2CPins.GPIO_PinConfig.GPIO_PinOPtype = GPIO_OP_TYPE_OD; // open drain
    I2CPins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU; // pull-up
    I2CPins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    // SCL
    I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6;
    GPIO_Init(&I2CPins);

    // SDA
    I2CPins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7;
    GPIO_Init(&I2CPins);
}

void I2C1_Inits(void)
{
    I2C1Handle.pI2Cx = I2C1;
    I2C1Handle.I2C_Config.I2C_AckControl = I2C_ACK_ENABLE;
    I2C1Handle.I2C_Config.I2C_DeviceAddress = 0x61; // master address (any)
    I2C1Handle.I2C_Config.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    I2C1Handle.I2C_Config.I2C_SCLSpeed = I2C_SCL_SPEED_SM; // 100kHz

    I2C_Init(&I2C1Handle);
}

#define SLAVE_ADDR 0x68
int main(void)
{
    char msg[] = "Hello ESP32";
    uint8_t rxBuffer[32];

    I2C1_GPIOInits();
    I2C1_Inits();

    // Enable I2C peripheral
    I2C_PeripheralControl(I2C1, ENABLE);
    I2C_ManageACKing(I2C1, ENABLE);

    while(1)
    {

        I2C_MasterSendData(&I2C1Handle, (uint8_t*)msg, strlen(msg),SLAVE_ADDR, I2C_DISABLE_SR);
        delay();
        I2C_MasterReceiveData(&I2C1Handle, rxBuffer, 11, SLAVE_ADDR, I2C_DISABLE_SR);
        delay();
    }
}
