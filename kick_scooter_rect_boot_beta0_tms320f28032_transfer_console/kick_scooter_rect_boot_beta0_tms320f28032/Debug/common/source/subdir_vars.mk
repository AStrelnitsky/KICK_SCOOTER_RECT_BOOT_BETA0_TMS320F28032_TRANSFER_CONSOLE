################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
ASM_SRCS += \
../common/source/F2802x_CSMPasswords.asm \
../common/source/F2802x_DBGIER.asm \
../common/source/F2802x_asmfuncs.asm 

C_SRCS += \
../common/source/F2802x_Adc.c \
../common/source/F2802x_Comp.c \
../common/source/F2802x_CpuTimers.c \
../common/source/F2802x_DefaultIsr.c \
../common/source/F2802x_ECap.c \
../common/source/F2802x_EPwm.c \
../common/source/F2802x_Gpio.c \
../common/source/F2802x_I2C.c \
../common/source/F2802x_OscComp.c \
../common/source/F2802x_PieCtrl.c \
../common/source/F2802x_PieVect.c \
../common/source/F2802x_Sci.c \
../common/source/F2802x_Spi.c \
../common/source/F2802x_SysCtrl.c \
../common/source/F2802x_TempSensorConv.c \
../common/source/adc.c \
../common/source/cap.c \
../common/source/clk.c \
../common/source/comp.c \
../common/source/cpu.c \
../common/source/flash.c \
../common/source/gpio.c \
../common/source/i2c.c \
../common/source/osc.c \
../common/source/pie.c \
../common/source/pll.c \
../common/source/pwm.c \
../common/source/pwr.c \
../common/source/sci.c \
../common/source/sci_io.c \
../common/source/spi.c \
../common/source/timer.c \
../common/source/wdog.c 

C_DEPS += \
./common/source/F2802x_Adc.d \
./common/source/F2802x_Comp.d \
./common/source/F2802x_CpuTimers.d \
./common/source/F2802x_DefaultIsr.d \
./common/source/F2802x_ECap.d \
./common/source/F2802x_EPwm.d \
./common/source/F2802x_Gpio.d \
./common/source/F2802x_I2C.d \
./common/source/F2802x_OscComp.d \
./common/source/F2802x_PieCtrl.d \
./common/source/F2802x_PieVect.d \
./common/source/F2802x_Sci.d \
./common/source/F2802x_Spi.d \
./common/source/F2802x_SysCtrl.d \
./common/source/F2802x_TempSensorConv.d \
./common/source/adc.d \
./common/source/cap.d \
./common/source/clk.d \
./common/source/comp.d \
./common/source/cpu.d \
./common/source/flash.d \
./common/source/gpio.d \
./common/source/i2c.d \
./common/source/osc.d \
./common/source/pie.d \
./common/source/pll.d \
./common/source/pwm.d \
./common/source/pwr.d \
./common/source/sci.d \
./common/source/sci_io.d \
./common/source/spi.d \
./common/source/timer.d \
./common/source/wdog.d 

OBJS += \
./common/source/F2802x_Adc.obj \
./common/source/F2802x_CSMPasswords.obj \
./common/source/F2802x_Comp.obj \
./common/source/F2802x_CpuTimers.obj \
./common/source/F2802x_DBGIER.obj \
./common/source/F2802x_DefaultIsr.obj \
./common/source/F2802x_ECap.obj \
./common/source/F2802x_EPwm.obj \
./common/source/F2802x_Gpio.obj \
./common/source/F2802x_I2C.obj \
./common/source/F2802x_OscComp.obj \
./common/source/F2802x_PieCtrl.obj \
./common/source/F2802x_PieVect.obj \
./common/source/F2802x_Sci.obj \
./common/source/F2802x_Spi.obj \
./common/source/F2802x_SysCtrl.obj \
./common/source/F2802x_TempSensorConv.obj \
./common/source/F2802x_asmfuncs.obj \
./common/source/adc.obj \
./common/source/cap.obj \
./common/source/clk.obj \
./common/source/comp.obj \
./common/source/cpu.obj \
./common/source/flash.obj \
./common/source/gpio.obj \
./common/source/i2c.obj \
./common/source/osc.obj \
./common/source/pie.obj \
./common/source/pll.obj \
./common/source/pwm.obj \
./common/source/pwr.obj \
./common/source/sci.obj \
./common/source/sci_io.obj \
./common/source/spi.obj \
./common/source/timer.obj \
./common/source/wdog.obj 

ASM_DEPS += \
./common/source/F2802x_CSMPasswords.d \
./common/source/F2802x_DBGIER.d \
./common/source/F2802x_asmfuncs.d 

OBJS__QUOTED += \
"common\source\F2802x_Adc.obj" \
"common\source\F2802x_CSMPasswords.obj" \
"common\source\F2802x_Comp.obj" \
"common\source\F2802x_CpuTimers.obj" \
"common\source\F2802x_DBGIER.obj" \
"common\source\F2802x_DefaultIsr.obj" \
"common\source\F2802x_ECap.obj" \
"common\source\F2802x_EPwm.obj" \
"common\source\F2802x_Gpio.obj" \
"common\source\F2802x_I2C.obj" \
"common\source\F2802x_OscComp.obj" \
"common\source\F2802x_PieCtrl.obj" \
"common\source\F2802x_PieVect.obj" \
"common\source\F2802x_Sci.obj" \
"common\source\F2802x_Spi.obj" \
"common\source\F2802x_SysCtrl.obj" \
"common\source\F2802x_TempSensorConv.obj" \
"common\source\F2802x_asmfuncs.obj" \
"common\source\adc.obj" \
"common\source\cap.obj" \
"common\source\clk.obj" \
"common\source\comp.obj" \
"common\source\cpu.obj" \
"common\source\flash.obj" \
"common\source\gpio.obj" \
"common\source\i2c.obj" \
"common\source\osc.obj" \
"common\source\pie.obj" \
"common\source\pll.obj" \
"common\source\pwm.obj" \
"common\source\pwr.obj" \
"common\source\sci.obj" \
"common\source\sci_io.obj" \
"common\source\spi.obj" \
"common\source\timer.obj" \
"common\source\wdog.obj" 

C_DEPS__QUOTED += \
"common\source\F2802x_Adc.d" \
"common\source\F2802x_Comp.d" \
"common\source\F2802x_CpuTimers.d" \
"common\source\F2802x_DefaultIsr.d" \
"common\source\F2802x_ECap.d" \
"common\source\F2802x_EPwm.d" \
"common\source\F2802x_Gpio.d" \
"common\source\F2802x_I2C.d" \
"common\source\F2802x_OscComp.d" \
"common\source\F2802x_PieCtrl.d" \
"common\source\F2802x_PieVect.d" \
"common\source\F2802x_Sci.d" \
"common\source\F2802x_Spi.d" \
"common\source\F2802x_SysCtrl.d" \
"common\source\F2802x_TempSensorConv.d" \
"common\source\adc.d" \
"common\source\cap.d" \
"common\source\clk.d" \
"common\source\comp.d" \
"common\source\cpu.d" \
"common\source\flash.d" \
"common\source\gpio.d" \
"common\source\i2c.d" \
"common\source\osc.d" \
"common\source\pie.d" \
"common\source\pll.d" \
"common\source\pwm.d" \
"common\source\pwr.d" \
"common\source\sci.d" \
"common\source\sci_io.d" \
"common\source\spi.d" \
"common\source\timer.d" \
"common\source\wdog.d" 

ASM_DEPS__QUOTED += \
"common\source\F2802x_CSMPasswords.d" \
"common\source\F2802x_DBGIER.d" \
"common\source\F2802x_asmfuncs.d" 

C_SRCS__QUOTED += \
"../common/source/F2802x_Adc.c" \
"../common/source/F2802x_Comp.c" \
"../common/source/F2802x_CpuTimers.c" \
"../common/source/F2802x_DefaultIsr.c" \
"../common/source/F2802x_ECap.c" \
"../common/source/F2802x_EPwm.c" \
"../common/source/F2802x_Gpio.c" \
"../common/source/F2802x_I2C.c" \
"../common/source/F2802x_OscComp.c" \
"../common/source/F2802x_PieCtrl.c" \
"../common/source/F2802x_PieVect.c" \
"../common/source/F2802x_Sci.c" \
"../common/source/F2802x_Spi.c" \
"../common/source/F2802x_SysCtrl.c" \
"../common/source/F2802x_TempSensorConv.c" \
"../common/source/adc.c" \
"../common/source/cap.c" \
"../common/source/clk.c" \
"../common/source/comp.c" \
"../common/source/cpu.c" \
"../common/source/flash.c" \
"../common/source/gpio.c" \
"../common/source/i2c.c" \
"../common/source/osc.c" \
"../common/source/pie.c" \
"../common/source/pll.c" \
"../common/source/pwm.c" \
"../common/source/pwr.c" \
"../common/source/sci.c" \
"../common/source/sci_io.c" \
"../common/source/spi.c" \
"../common/source/timer.c" \
"../common/source/wdog.c" 

ASM_SRCS__QUOTED += \
"../common/source/F2802x_CSMPasswords.asm" \
"../common/source/F2802x_DBGIER.asm" \
"../common/source/F2802x_asmfuncs.asm" 


