#This file is generated by VisualGDB.
#It contains GCC settings automatically derived from the board support package (BSP).
#DO NOT EDIT MANUALLY. THE FILE WILL BE OVERWRITTEN. 
#Use VisualGDB Project Properties dialog or modify Makefile or per-configuration .mak files instead.

#In order to build this project manually (outside VisualGDB), please provide TOOLCHAIN_ROOT, BSP_ROOT, EFP_BASE and TESTFW_BASE variables via Environment or Make command line
#Embedded toolchain
CC := $(TOOLCHAIN_ROOT)/bin/arm-eabi-gcc.exe
CXX := $(TOOLCHAIN_ROOT)/bin/arm-eabi-g++.exe
LD := $(CXX)
AR := $(TOOLCHAIN_ROOT)/bin/arm-eabi-ar.exe
OBJCOPY := $(TOOLCHAIN_ROOT)/bin/arm-eabi-objcopy.exe

#Additional flags
PREPROCESSOR_MACROS += NRF52832_XXAA nosoftdev NRF_DFU_SETTINGS_VERSION=1 NRF_SD_BLE_API_VERSION=5 ARM_MATH_CM4 NRF52 SWI_DISABLE0
INCLUDE_DIRS += . $(BSP_ROOT)/nRF5x/components/softdevice/nosoftdev/headers $(BSP_ROOT)/nRF5x/components/softdevice/nosoftdev/headers/nrf52 $(BSP_ROOT)/nRF5x/components/toolchain $(BSP_ROOT)/nRF5x/components/toolchain/cmsis/include $(BSP_ROOT)/nRF5x/components/device
LIBRARY_DIRS += $(BSP_ROOT)/nRF5x/SoftdeviceLibraries $(BSP_ROOT)/nRF5x/LinkerScripts
LIBRARY_NAMES += 
ADDITIONAL_LINKER_INPUTS += 
MACOS_FRAMEWORKS += 
LINUX_PACKAGES += 

CFLAGS += -std=gnu99 
CXXFLAGS += 
ASFLAGS += -mfpu=fpv4-sp-d16
LDFLAGS += --specs=nano.specs --specs=nosys.specs
COMMONFLAGS += -mabi=aapcs  -mcpu=cortex-m4 -mthumb -mfloat-abi=hard
LINKER_SCRIPT := $(BSP_ROOT)/nRF5x/LinkerScripts/nRF52832_XXAA_nosoftdev.lds

