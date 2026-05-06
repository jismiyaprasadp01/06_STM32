################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Driver/src/sdcard.c \
../Driver/src/stm32f407xx_gpio._driver.c \
../Driver/src/stm32f407xx_spi_driver.c 

OBJS += \
./Driver/src/sdcard.o \
./Driver/src/stm32f407xx_gpio._driver.o \
./Driver/src/stm32f407xx_spi_driver.o 

C_DEPS += \
./Driver/src/sdcard.d \
./Driver/src/stm32f407xx_gpio._driver.d \
./Driver/src/stm32f407xx_spi_driver.d 


# Each subdirectory must supply rules for building sources it contributes
Driver/src/%.o Driver/src/%.su Driver/src/%.cyclo: ../Driver/src/%.c Driver/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -I"C:/STM32/20_SPI_Driver/Driver/inc" -I"C:/STM32/20_SPI_Driver/Driver/src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Driver-2f-src

clean-Driver-2f-src:
	-$(RM) ./Driver/src/sdcard.cyclo ./Driver/src/sdcard.d ./Driver/src/sdcard.o ./Driver/src/sdcard.su ./Driver/src/stm32f407xx_gpio._driver.cyclo ./Driver/src/stm32f407xx_gpio._driver.d ./Driver/src/stm32f407xx_gpio._driver.o ./Driver/src/stm32f407xx_gpio._driver.su ./Driver/src/stm32f407xx_spi_driver.cyclo ./Driver/src/stm32f407xx_spi_driver.d ./Driver/src/stm32f407xx_spi_driver.o ./Driver/src/stm32f407xx_spi_driver.su

.PHONY: clean-Driver-2f-src

