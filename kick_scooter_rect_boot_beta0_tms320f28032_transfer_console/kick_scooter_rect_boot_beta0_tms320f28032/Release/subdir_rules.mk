################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.5.LTS/bin/cl2000" -v28 -ml -mt -O0 --opt_for_speed=2 --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.5.LTS/include" --include_path="C:/ti/c2000/C2000Ware_3_03_00_00/libraries/flash_api/f2802x/include" --include_path="C:/Workbanch_NEW/Workbanch_NEW/WT/kick_scooter_rect_boot_beta0/kick_scooter_rect_boot_beta0" --advice:performance=all --define=_FLASH --define=_DEBUG --c99 --c++03 --relaxed_ansi --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


