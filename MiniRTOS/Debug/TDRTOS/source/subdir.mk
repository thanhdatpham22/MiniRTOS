################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../TDRTOS/source/scheduler.c \
../TDRTOS/source/task.c 

OBJS += \
./TDRTOS/source/scheduler.o \
./TDRTOS/source/task.o 

C_DEPS += \
./TDRTOS/source/scheduler.d \
./TDRTOS/source/task.d 


# Each subdirectory must supply rules for building sources it contributes
TDRTOS/source/%.o TDRTOS/source/%.su TDRTOS/source/%.cyclo: ../TDRTOS/source/%.c TDRTOS/source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I"C:/Users/admin/STM32CubeIDE/workspace_1.16.0/TDRTOS/TDRTOS/source" -I"C:/Users/admin/STM32CubeIDE/workspace_1.16.0/TDRTOS/TDRTOS/include" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-TDRTOS-2f-source

clean-TDRTOS-2f-source:
	-$(RM) ./TDRTOS/source/scheduler.cyclo ./TDRTOS/source/scheduler.d ./TDRTOS/source/scheduler.o ./TDRTOS/source/scheduler.su ./TDRTOS/source/task.cyclo ./TDRTOS/source/task.d ./TDRTOS/source/task.o ./TDRTOS/source/task.su

.PHONY: clean-TDRTOS-2f-source

