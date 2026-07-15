################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/application/app.c 

OBJS += \
./Core/Src/application/app.o 

C_DEPS += \
./Core/Src/application/app.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/application/%.o Core/Src/application/%.su Core/Src/application/%.cyclo: ../Core/Src/application/%.c Core/Src/application/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I"C:/Users/admin/STM32CubeIDE/workspace_1.16.0/TDRTOS/TDRTOS/source" -I"C:/Users/admin/STM32CubeIDE/workspace_1.16.0/TDRTOS/TDRTOS/include" -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-application

clean-Core-2f-Src-2f-application:
	-$(RM) ./Core/Src/application/app.cyclo ./Core/Src/application/app.d ./Core/Src/application/app.o ./Core/Src/application/app.su

.PHONY: clean-Core-2f-Src-2f-application

