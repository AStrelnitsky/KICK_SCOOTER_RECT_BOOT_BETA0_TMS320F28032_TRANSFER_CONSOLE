################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
common/source/%.obj: ../common/source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.5.LTS/bin/cl2000" -v28 -ml -mt --include_path="C:/Workbanch_NEW/Workbanch_NEW/WT/kick_scooter_rect_boot_beta0/kick_scooter_rect_boot_beta0" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.5.LTS/include" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.2.LTS/include" --include_path="/common/include" --include_path="/headers/include" --include_path="/include" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.5.LTS/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="common/source/$(basename $(<F)).d_raw" --obj_directory="common/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

common/source/%.obj: ../common/source/%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.5.LTS/bin/cl2000" -v28 -ml -mt --include_path="C:/Workbanch_NEW/Workbanch_NEW/WT/kick_scooter_rect_boot_beta0/kick_scooter_rect_boot_beta0" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.5.LTS/include" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.2.LTS/include" --include_path="/common/include" --include_path="/headers/include" --include_path="/include" --include_path="C:/ti/ccs1020/ccs/tools/compiler/ti-cgt-c2000_20.2.5.LTS/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --abi=coffabi --preproc_with_compile --preproc_dependency="common/source/$(basename $(<F)).d_raw" --obj_directory="common/source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


