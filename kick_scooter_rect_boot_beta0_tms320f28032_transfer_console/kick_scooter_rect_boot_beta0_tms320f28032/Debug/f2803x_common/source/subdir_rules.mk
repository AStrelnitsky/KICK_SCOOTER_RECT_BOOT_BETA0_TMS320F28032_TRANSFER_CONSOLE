################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
f2803x_common/source/%.obj: ../f2803x_common/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.2.LTS/bin/cl2000" -v28 -ml -mt --float_support=softlib -O3 --opt_for_speed=2 --fp_mode=strict --include_path="C:/Workbanch_NEW/Workbanch_NEW/WT/SVN/KICK_SCOOTER_RECT_BOOT_BETA0_TMS320F28032_TRANSFER_CONSOLE/kick_scooter_rect_boot_beta0_tms320f28032_transfer_console/kick_scooter_rect_boot_beta0_tms320f28032" --include_path="C:/ti/c2000/C2000Ware_3_03_00_00/libraries/flash_api/f2803x/include" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.2.LTS/include" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.2.LTS/include" --include_path="/common/include" --include_path="/headers/include" --include_path="C:/ti/C2000Ware_MotorControl_SDK_3_00_01_00/c2000ware/libraries/math/IQmath/c28/include" --include_path="C:/Workbanch_NEW/Workbanch_NEW/WT/SVN/KICK_SCOOTER_RECT_BOOT_BETA0_TMS320F28032_TRANSFER_CONSOLE/kick_scooter_rect_boot_beta0_tms320f28032_transfer_console/kick_scooter_rect_boot_beta0_tms320f28032/f2803x_common/include" --advice:performance=all --define=_INLINE --define=TMS320F2803x -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="f2803x_common/source/$(basename $(<F)).d_raw" --obj_directory="f2803x_common/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

f2803x_common/source/%.obj: ../f2803x_common/source/%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.2.LTS/bin/cl2000" -v28 -ml -mt --float_support=softlib -O3 --opt_for_speed=2 --fp_mode=strict --include_path="C:/Workbanch_NEW/Workbanch_NEW/WT/SVN/KICK_SCOOTER_RECT_BOOT_BETA0_TMS320F28032_TRANSFER_CONSOLE/kick_scooter_rect_boot_beta0_tms320f28032_transfer_console/kick_scooter_rect_boot_beta0_tms320f28032" --include_path="C:/ti/c2000/C2000Ware_3_03_00_00/libraries/flash_api/f2803x/include" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.2.LTS/include" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.2.LTS/include" --include_path="/common/include" --include_path="/headers/include" --include_path="C:/ti/C2000Ware_MotorControl_SDK_3_00_01_00/c2000ware/libraries/math/IQmath/c28/include" --include_path="C:/Workbanch_NEW/Workbanch_NEW/WT/SVN/KICK_SCOOTER_RECT_BOOT_BETA0_TMS320F28032_TRANSFER_CONSOLE/kick_scooter_rect_boot_beta0_tms320f28032_transfer_console/kick_scooter_rect_boot_beta0_tms320f28032/f2803x_common/include" --advice:performance=all --define=_INLINE --define=TMS320F2803x -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="f2803x_common/source/$(basename $(<F)).d_raw" --obj_directory="f2803x_common/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


