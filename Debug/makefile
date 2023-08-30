################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/subdir.mk
-include FreeRTOS_src/subdir.mk
-include FreeRTOS_portable/subdir.mk
-include FreeRTOS_portable/MemMang/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: sonometro.axf

# Tool invocations
sonometro.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"D:\UTN\Técnicas Digitales II\proyecto\soft\sonometro\Debug" -L"D:\UTN\Técnicas Digitales II\proyecto\soft\Lib_MCU_LPC176x\Debug" -L"D:\UTN\Técnicas Digitales II\proyecto\soft\Lib_API_CAPI\Debug" -L"D:\UTN\Técnicas Digitales II\proyecto\soft\CMSISv2p00_LPC17xx\Debug" -Xlinker -Map="sonometro.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "sonometro_Debug.ld" -o"sonometro.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) sonometro.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "sonometro.axf"; # arm-none-eabi-objcopy -O binary "sonometro.axf" "sonometro.bin" ; checksum -p LPC1769 -d "sonometro.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
