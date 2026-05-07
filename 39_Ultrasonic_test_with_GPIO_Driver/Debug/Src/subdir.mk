################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/ultrasonic_interrupt_print.c 

OBJS += \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/ultrasonic_interrupt_print.o 

C_DEPS += \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/ultrasonic_interrupt_print.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F407G_DISC1 -DSTM32F4 -DSTM32F407VGTx -c -I../Inc -I"C:/STM32/004_ultrasonic_distance_print/driver/inc" -I"C:/STM32/004_ultrasonic_distance_print/driver/src" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/ultrasonic_interrupt_print.cyclo ./Src/ultrasonic_interrupt_print.d ./Src/ultrasonic_interrupt_print.o ./Src/ultrasonic_interrupt_print.su

.PHONY: clean-Src

