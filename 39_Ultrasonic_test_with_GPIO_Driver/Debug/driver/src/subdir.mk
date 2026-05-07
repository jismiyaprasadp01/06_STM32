################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../driver/src/stm32f407xx_gpio._driver.c 

OBJS += \
./driver/src/stm32f407xx_gpio._driver.o 

C_DEPS += \
./driver/src/stm32f407xx_gpio._driver.d 


# Each subdirectory must supply rules for building sources it contributes
driver/src/%.o driver/src/%.su driver/src/%.cyclo: ../driver/src/%.c driver/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -I"C:/STM32/004_ultrasonic_distance_print/driver/inc" -I"C:/STM32/004_ultrasonic_distance_print/driver/src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-driver-2f-src

clean-driver-2f-src:
	-$(RM) ./driver/src/stm32f407xx_gpio._driver.cyclo ./driver/src/stm32f407xx_gpio._driver.d ./driver/src/stm32f407xx_gpio._driver.o ./driver/src/stm32f407xx_gpio._driver.su

.PHONY: clean-driver-2f-src

