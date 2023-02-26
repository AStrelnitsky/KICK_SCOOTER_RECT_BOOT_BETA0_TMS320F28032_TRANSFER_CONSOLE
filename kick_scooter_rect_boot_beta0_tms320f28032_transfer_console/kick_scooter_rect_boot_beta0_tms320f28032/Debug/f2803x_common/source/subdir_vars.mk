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
../f2803x_common/source/DSP2803x_Adc.c \
../f2803x_common/source/DSP2803x_Comp.c \
../f2803x_common/source/DSP2803x_CpuTimers.c \
../f2803x_common/source/DSP2803x_DefaultIsr.c \
../f2803x_common/source/DSP2803x_ECan.c \
../f2803x_common/source/DSP2803x_ECap.c \
../f2803x_common/source/DSP2803x_EPwm.c \
../f2803x_common/source/DSP2803x_EQep.c \
../f2803x_common/source/DSP2803x_Gpio.c \
../f2803x_common/source/DSP2803x_HRCap.c \
../f2803x_common/source/DSP2803x_I2C.c \
../f2803x_common/source/DSP2803x_Lin.c \
../f2803x_common/source/DSP2803x_OscComp.c \
../f2803x_common/source/DSP2803x_PieCtrl.c \
../f2803x_common/source/DSP2803x_PieVect.c \
../f2803x_common/source/DSP2803x_Sci.c \
../f2803x_common/source/DSP2803x_Spi.c \
../f2803x_common/source/DSP2803x_SysCtrl.c \
../f2803x_common/source/DSP2803x_TempSensorConv.c \
../f2803x_common/source/bootloader.c \
../f2803x_common/source/esp8266.c 

C_DEPS += \
./f2803x_common/source/DSP2803x_Adc.d \
./f2803x_common/source/DSP2803x_Comp.d \
./f2803x_common/source/DSP2803x_CpuTimers.d \
./f2803x_common/source/DSP2803x_DefaultIsr.d \
./f2803x_common/source/DSP2803x_ECan.d \
./f2803x_common/source/DSP2803x_ECap.d \
./f2803x_common/source/DSP2803x_EPwm.d \
./f2803x_common/source/DSP2803x_EQep.d \
./f2803x_common/source/DSP2803x_Gpio.d \
./f2803x_common/source/DSP2803x_HRCap.d \
./f2803x_common/source/DSP2803x_I2C.d \
./f2803x_common/source/DSP2803x_Lin.d \
./f2803x_common/source/DSP2803x_OscComp.d \
./f2803x_common/source/DSP2803x_PieCtrl.d \
./f2803x_common/source/DSP2803x_PieVect.d \
./f2803x_common/source/DSP2803x_Sci.d \
./f2803x_common/source/DSP2803x_Spi.d \
./f2803x_common/source/DSP2803x_SysCtrl.d \
./f2803x_common/source/DSP2803x_TempSensorConv.d \
./f2803x_common/source/bootloader.d \
./f2803x_common/source/esp8266.d 

OBJS += \
./f2803x_common/source/DSP2803x_Adc.obj \
./f2803x_common/source/DSP2803x_CSMPasswords.obj \
./f2803x_common/source/DSP2803x_CodeStartBranch.obj \
./f2803x_common/source/DSP2803x_Comp.obj \
./f2803x_common/source/DSP2803x_CpuTimers.obj \
./f2803x_common/source/DSP2803x_DBGIER.obj \
./f2803x_common/source/DSP2803x_DefaultIsr.obj \
./f2803x_common/source/DSP2803x_DisInt.obj \
./f2803x_common/source/DSP2803x_ECan.obj \
./f2803x_common/source/DSP2803x_ECap.obj \
./f2803x_common/source/DSP2803x_EPwm.obj \
./f2803x_common/source/DSP2803x_EQep.obj \
./f2803x_common/source/DSP2803x_Gpio.obj \
./f2803x_common/source/DSP2803x_HRCap.obj \
./f2803x_common/source/DSP2803x_I2C.obj \
./f2803x_common/source/DSP2803x_Lin.obj \
./f2803x_common/source/DSP2803x_OscComp.obj \
./f2803x_common/source/DSP2803x_PieCtrl.obj \
./f2803x_common/source/DSP2803x_PieVect.obj \
./f2803x_common/source/DSP2803x_Sci.obj \
./f2803x_common/source/DSP2803x_Spi.obj \
./f2803x_common/source/DSP2803x_SysCtrl.obj \
./f2803x_common/source/DSP2803x_TempSensorConv.obj \
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
"f2803x_common\source\DSP2803x_Adc.obj" \
"f2803x_common\source\DSP2803x_CSMPasswords.obj" \
"f2803x_common\source\DSP2803x_CodeStartBranch.obj" \
"f2803x_common\source\DSP2803x_Comp.obj" \
"f2803x_common\source\DSP2803x_CpuTimers.obj" \
"f2803x_common\source\DSP2803x_DBGIER.obj" \
"f2803x_common\source\DSP2803x_DefaultIsr.obj" \
"f2803x_common\source\DSP2803x_DisInt.obj" \
"f2803x_common\source\DSP2803x_ECan.obj" \
"f2803x_common\source\DSP2803x_ECap.obj" \
"f2803x_common\source\DSP2803x_EPwm.obj" \
"f2803x_common\source\DSP2803x_EQep.obj" \
"f2803x_common\source\DSP2803x_Gpio.obj" \
"f2803x_common\source\DSP2803x_HRCap.obj" \
"f2803x_common\source\DSP2803x_I2C.obj" \
"f2803x_common\source\DSP2803x_Lin.obj" \
"f2803x_common\source\DSP2803x_OscComp.obj" \
"f2803x_common\source\DSP2803x_PieCtrl.obj" \
"f2803x_common\source\DSP2803x_PieVect.obj" \
"f2803x_common\source\DSP2803x_Sci.obj" \
"f2803x_common\source\DSP2803x_Spi.obj" \
"f2803x_common\source\DSP2803x_SysCtrl.obj" \
"f2803x_common\source\DSP2803x_TempSensorConv.obj" \
"f2803x_common\source\DSP2803x_usDelay.obj" \
"f2803x_common\source\bootloader.obj" \
"f2803x_common\source\esp8266.obj" 

C_DEPS__QUOTED += \
"f2803x_common\source\DSP2803x_Adc.d" \
"f2803x_common\source\DSP2803x_Comp.d" \
"f2803x_common\source\DSP2803x_CpuTimers.d" \
"f2803x_common\source\DSP2803x_DefaultIsr.d" \
"f2803x_common\source\DSP2803x_ECan.d" \
"f2803x_common\source\DSP2803x_ECap.d" \
"f2803x_common\source\DSP2803x_EPwm.d" \
"f2803x_common\source\DSP2803x_EQep.d" \
"f2803x_common\source\DSP2803x_Gpio.d" \
"f2803x_common\source\DSP2803x_HRCap.d" \
"f2803x_common\source\DSP2803x_I2C.d" \
"f2803x_common\source\DSP2803x_Lin.d" \
"f2803x_common\source\DSP2803x_OscComp.d" \
"f2803x_common\source\DSP2803x_PieCtrl.d" \
"f2803x_common\source\DSP2803x_PieVect.d" \
"f2803x_common\source\DSP2803x_Sci.d" \
"f2803x_common\source\DSP2803x_Spi.d" \
"f2803x_common\source\DSP2803x_SysCtrl.d" \
"f2803x_common\source\DSP2803x_TempSensorConv.d" \
"f2803x_common\source\bootloader.d" \
"f2803x_common\source\esp8266.d" 

ASM_DEPS__QUOTED += \
"f2803x_common\source\DSP2803x_CSMPasswords.d" \
"f2803x_common\source\DSP2803x_CodeStartBranch.d" \
"f2803x_common\source\DSP2803x_DBGIER.d" \
"f2803x_common\source\DSP2803x_DisInt.d" \
"f2803x_common\source\DSP2803x_usDelay.d" 

C_SRCS__QUOTED += \
"../f2803x_common/source/DSP2803x_Adc.c" \
"../f2803x_common/source/DSP2803x_Comp.c" \
"../f2803x_common/source/DSP2803x_CpuTimers.c" \
"../f2803x_common/source/DSP2803x_DefaultIsr.c" \
"../f2803x_common/source/DSP2803x_ECan.c" \
"../f2803x_common/source/DSP2803x_ECap.c" \
"../f2803x_common/source/DSP2803x_EPwm.c" \
"../f2803x_common/source/DSP2803x_EQep.c" \
"../f2803x_common/source/DSP2803x_Gpio.c" \
"../f2803x_common/source/DSP2803x_HRCap.c" \
"../f2803x_common/source/DSP2803x_I2C.c" \
"../f2803x_common/source/DSP2803x_Lin.c" \
"../f2803x_common/source/DSP2803x_OscComp.c" \
"../f2803x_common/source/DSP2803x_PieCtrl.c" \
"../f2803x_common/source/DSP2803x_PieVect.c" \
"../f2803x_common/source/DSP2803x_Sci.c" \
"../f2803x_common/source/DSP2803x_Spi.c" \
"../f2803x_common/source/DSP2803x_SysCtrl.c" \
"../f2803x_common/source/DSP2803x_TempSensorConv.c" \
"../f2803x_common/source/bootloader.c" \
"../f2803x_common/source/esp8266.c" 

ASM_SRCS__QUOTED += \
"../f2803x_common/source/DSP2803x_CSMPasswords.asm" \
"../f2803x_common/source/DSP2803x_CodeStartBranch.asm" \
"../f2803x_common/source/DSP2803x_DBGIER.asm" \
"../f2803x_common/source/DSP2803x_DisInt.asm" \
"../f2803x_common/source/DSP2803x_usDelay.asm" 


