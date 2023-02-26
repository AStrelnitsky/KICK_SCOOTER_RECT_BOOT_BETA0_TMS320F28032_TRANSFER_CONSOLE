################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../DSP2803x_Headers_BIOS.cmd \
../DSP2803x_Headers_nonBIOS.cmd \
../F2802x_Headers_nonBIOS.cmd \
../F2802x_generic_flash.cmd \
../F28032.cmd \
C:/Workbanch_NEW/Workbanch_NEW/WT/SVN/KICK_SCOOTER_RECT_BOOT_BETA0_TMS320F28032_TRANSFER_CONSOLE/kick_scooter_rect_beta0_tms320f28032/kick_scooter_rect_beta0_tms320f28032/F28032_boot.cmd 

LIB_SRCS += \
../2802x_FlashAPI_BootROMSymbols_v2.01.lib \
../2803x_FlashAPI_BootROMSymbols.lib \
../Flash2802x_API_V201.lib \
../Flash2803x_API_V100.lib 

ASM_SRCS += \
../Exit_Boot.asm 

S_UPPER_SRCS += \
../F2802x_CodeStartBranch.S 

C_SRCS += \
../DSP2803x_GlobalVariableDefs.c \
../DSP28xx_SciUtil.c \
../F2802x_GlobalVariableDefs.c \
../bootloader.c \
../main.c 

C_DEPS += \
./DSP2803x_GlobalVariableDefs.d \
./DSP28xx_SciUtil.d \
./F2802x_GlobalVariableDefs.d \
./bootloader.d \
./main.d 

OBJS += \
./DSP2803x_GlobalVariableDefs.obj \
./DSP28xx_SciUtil.obj \
./Exit_Boot.obj \
./F2802x_CodeStartBranch.obj \
./F2802x_GlobalVariableDefs.obj \
./bootloader.obj \
./main.obj 

ASM_DEPS += \
./Exit_Boot.d 

S_UPPER_DEPS += \
./F2802x_CodeStartBranch.d 

S_UPPER_DEPS__QUOTED += \
"F2802x_CodeStartBranch.d" 

OBJS__QUOTED += \
"DSP2803x_GlobalVariableDefs.obj" \
"DSP28xx_SciUtil.obj" \
"Exit_Boot.obj" \
"F2802x_CodeStartBranch.obj" \
"F2802x_GlobalVariableDefs.obj" \
"bootloader.obj" \
"main.obj" 

C_DEPS__QUOTED += \
"DSP2803x_GlobalVariableDefs.d" \
"DSP28xx_SciUtil.d" \
"F2802x_GlobalVariableDefs.d" \
"bootloader.d" \
"main.d" 

ASM_DEPS__QUOTED += \
"Exit_Boot.d" 

C_SRCS__QUOTED += \
"../DSP2803x_GlobalVariableDefs.c" \
"../DSP28xx_SciUtil.c" \
"../F2802x_GlobalVariableDefs.c" \
"../bootloader.c" \
"../main.c" 

ASM_SRCS__QUOTED += \
"../Exit_Boot.asm" 

S_UPPER_SRCS__QUOTED += \
"../F2802x_CodeStartBranch.S" 


