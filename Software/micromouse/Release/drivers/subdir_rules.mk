################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
drivers/bluetooth.obj: ../drivers/bluetooth.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/ti/tirtos_1_21_00_09/products/TivaWare_C_Series-2.0.1.11577a" --gcc --define=PART_TM4C123GH6PM --define=ccs --define=TIVAWARE --diag_warning=225 --display_error_number --diag_wrap=off --gen_func_subsections=on --preproc_with_compile --preproc_dependency="drivers/bluetooth.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/control.obj: ../drivers/control.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/ti/tirtos_1_21_00_09/products/TivaWare_C_Series-2.0.1.11577a" --gcc --define=PART_TM4C123GH6PM --define=ccs --define=TIVAWARE --diag_warning=225 --display_error_number --diag_wrap=off --gen_func_subsections=on --preproc_with_compile --preproc_dependency="drivers/control.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/encoder.obj: ../drivers/encoder.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/ti/tirtos_1_21_00_09/products/TivaWare_C_Series-2.0.1.11577a" --gcc --define=PART_TM4C123GH6PM --define=ccs --define=TIVAWARE --diag_warning=225 --display_error_number --diag_wrap=off --gen_func_subsections=on --preproc_with_compile --preproc_dependency="drivers/encoder.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/ir_sensor.obj: ../drivers/ir_sensor.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/ti/tirtos_1_21_00_09/products/TivaWare_C_Series-2.0.1.11577a" --gcc --define=PART_TM4C123GH6PM --define=ccs --define=TIVAWARE --diag_warning=225 --display_error_number --diag_wrap=off --gen_func_subsections=on --preproc_with_compile --preproc_dependency="drivers/ir_sensor.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/led.obj: ../drivers/led.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/ti/tirtos_1_21_00_09/products/TivaWare_C_Series-2.0.1.11577a" --gcc --define=PART_TM4C123GH6PM --define=ccs --define=TIVAWARE --diag_warning=225 --display_error_number --diag_wrap=off --gen_func_subsections=on --preproc_with_compile --preproc_dependency="drivers/led.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/motor.obj: ../drivers/motor.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccsv5/tools/compiler/arm_5.1.1/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me -O2 --include_path="C:/ti/ccsv5/tools/compiler/arm_5.1.1/include" --include_path="C:/ti/tirtos_1_21_00_09/products/TivaWare_C_Series-2.0.1.11577a" --gcc --define=PART_TM4C123GH6PM --define=ccs --define=TIVAWARE --diag_warning=225 --display_error_number --diag_wrap=off --gen_func_subsections=on --preproc_with_compile --preproc_dependency="drivers/motor.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


