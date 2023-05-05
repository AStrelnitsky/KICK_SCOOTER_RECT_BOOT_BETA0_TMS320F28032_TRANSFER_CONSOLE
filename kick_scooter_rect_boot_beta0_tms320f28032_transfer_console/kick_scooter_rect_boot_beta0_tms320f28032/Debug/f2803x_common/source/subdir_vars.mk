################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
ASM_SRCS += \
../f2803x_common/source/DSP2803x_CSMPasswords.asm \
../f2803x_common/source/DSP2803x_CodeStartBranch.asm \
../f2803x_common/source/DSP2803x_DBGIER.asm \
../f2803x_common/source/DSP2803x_DisInt.asm \
../f2803x_common/source/DSP2803x_usDelay.asm 

C_SRCS += \
../f2803x_common/source/DSP2803x_CpuTimers.c \
../f2803x_common/source/DSP2803x_DefaultIsr.c \
../f2803x_common/source/DSP2803x_Gpio.c \
../f2803x_common/source/DSP2803x_OscComp.c \
../f2803x_common/source/DSP2803x_PieCtrl.c \
../f2803x_common/source/DSP2803x_PieVect.c \
../f2803x_common/source/DSP2803x_Sci.c \
../f2803x_common/source/DSP2803x_SysCtrl.c \
../f2803x_common/source/bootloader.c \
../f2803x_common/source/esp8266.c 

C_DEPS += \
./f2803x_common/source/DSP2803x_CpuTimers.d \
./f2803x_common/source/DSP2803x_DefaultIsr.d \
./f2803x_common/source/DSP2803x_Gpio.d \
./f2803x_common/source/DSP2803x_OscComp.d \
./f2803x_common/source/DSP2803x_PieCtrl.d \
./f2803x_common/source/DSP2803x_PieVect.d \
./f2803x_common/source/DSP2803x_Sci.d \
./f2803x_common/source/DSP2803x_SysCtrl.d \
./f2803x_common/source/bootloader.d \
./f2803x_common/source/esp8266.d 

OBJS += \
./f2803x_common/source/DSP2803x_CSMPasswords.obj \
./f2803x_common/source/DSP2803x_CodeStartBranch.obj \
./f2803x_common/source/DSP2803x_CpuTimers.obj \
./f2803x_common/source/DSP2803x_DBGIER.obj \
./f2803x_common/source/DSP2803x_DefaultIsr.obj \
./f2803x_common/source/DSP2803x_DisInt.obj \
./f2803x_common/source/DSP2803x_Gpio.obj \
./f2803x_common/source/DSP2803x_OscComp.obj \
./f2803x_common/source/DSP2803x_PieCtrl.obj \
./f2803x_common/source/DSP2803x_PieVect.obj \
./f2803x_common/source/DSP2803x_Sci.obj \
./f2803x_common/source/DSP2803x_SysCtrl.obj \
./f2803x_common/source/DSP2803x_usDelay.obj \
./f2803x_common/source/bootloader.obj \
./f2803x_common/source/esp8266.obj 

ASM_DEPS += \
./f2803x_common/source/DSP2803x_CSMPasswords.d \
./f2803x_common/source/DSP2803x_CodeStartBranch.d \
./f2803x_common/source/DSP2803x_DBGIER.d \
./f2803x_common/source/DSP2803x_DisInt.d \
./f2803x_common/source/DSP2803x_usDelay.d 

OBJS__QUOTED += \
"f2803x_common\source\DSP2803x_CSMPasswords.obj" \
"f2803x_common\source\DSP2803x_CodeStartBranch.obj" \
"f2803x_common\source\DSP2803x_CpuTimers.obj" \
"f2803x_common\source\DSP2803x_DBGIER.obj" \
"f2803x_common\source\DSP2803x_DefaultIsr.obj" \
"f2803x_common\source\DSP2803x_DisInt.obj" \
"f2803x_common\source\DSP2803x_Gpio.obj" \
"f2803x_common\source\DSP2803x_OscComp.obj" \
"f2803x_common\source\DSP2803x_PieCtrl.obj" \
"f2803x_common\source\DSP2803x_PieVect.obj" \
"f2803x_common\source\DSP2803x_Sci.obj" \
"f2803x_common\source\DSP2803x_SysCtrl.obj" \
"f2803x_common\source\DSP2803x_usDelay.obj" \
"f2803x_common\source\bootloader.obj" \
"f2803x_common\source\esp8266.obj" 

C_DEPS__QUOTED += \
"f2803x_common\source\DSP2803x_CpuTimers.d" \
"f2803x_common\source\DSP2803x_DefaultIsr.d" \
"f2803x_common\source\DSP2803x_Gpio.d" \
"f2803x_common\source\DSP2803x_OscComp.d" \
"f2803x_common\source\DSP2803x_PieCtrl.d" \
"f2803x_common\source\DSP2803x_PieVect.d" \
"f2803x_common\source\DSP2803x_Sci.d" \
"f2803x_common\source\DSP2803x_SysCtrl.d" \
"f2803x_common\source\bootloader.d" \
"f2803x_common\source\esp8266.d" 

ASM_DEPS__QUOTED += \
"f2803x_common\source\DSP2803x_CSMPasswords.d" \
"f2803x_common\source\DSP2803x_CodeStartBranch.d" \
"f2803x_common\source\DSP2803x_DBGIER.d" \
"f2803x_common\source\DSP2803x_DisInt.d" \
"f2803x_common\source\DSP2803x_usDelay.d" 

ASM_SRCS__QUOTED += \
"../f2803x_common/source/DSP2803x_CSMPasswords.asm" \
"../f2803x_common/source/DSP2803x_CodeStartBranch.asm" \
"../f2803x_common/source/DSP2803x_DBGIER.asm" \
"../f2803x_common/source/DSP2803x_DisInt.asm" \
"../f2803x_common/source/DSP2803x_usDelay.asm" 

C_SRCS__QUOTED += \
"../f2803x_common/source/DSP2803x_CpuTimers.c" \
"../f2803x_common/source/DSP2803x_DefaultIsr.c" \
"../f2803x_common/source/DSP2803x_Gpio.c" \
"../f2803x_common/source/DSP2803x_OscComp.c" \
"../f2803x_common/source/DSP2803x_PieCtrl.c" \
"../f2803x_common/source/DSP2803x_PieVect.c" \
"../f2803x_common/source/DSP2803x_Sci.c" \
"../f2803x_common/source/DSP2803x_SysCtrl.c" \
"../f2803x_common/source/bootloader.c" \
"../f2803x_common/source/esp8266.c" 


