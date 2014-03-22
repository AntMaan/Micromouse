/*
 * control.c
 *
 *  Created on: Mar 21, 2014
 *      Author: Anthony
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>

#include "control.h"
#include "system.h"
#include "drivers/motor.h"
#include "drivers/bluetooth.h"
#include "drivers/ir_sensor.h"
#include "services/pid.h"
#include "services/time_keeper.h"

#include <ti/drivers/GPIO.h>
#include <inc/hw_ints.h>
#include <inc/hw_gpio.h>


#include <xdc/runtime/System.h>

typedef struct straight_pid_params_t {
	float kp;
	float ki;
	float kd;
	uint32_t motor_speed;
}straight_pid_params_t;

straight_pid_params_t straight_control_params = {.00001, 0, 0, 500};

Semaphore_Handle drive_straight_sem_handle;
Semaphore_Params drive_straight_sem_params;

pid_controller_t side_pid;

void drive_straight(){

	side_ir_data_t side_data;
	uint32_t left_avg;
	uint32_t right_avg;
	int32_t side_diff;

	float motor_diff;

	int16_t right_motor_out;
	int16_t left_motor_out;


	while(1){
		Semaphore_pend(drive_straight_sem_handle, BIOS_WAIT_FOREVER);

		side_poll(&side_data);

		left_avg = (side_data.left_back + side_data.left_front)/2;
		right_avg = (side_data.right_back + side_data.right_front)/2;

		side_diff = left_avg - right_avg;

		motor_diff = pid_step(&side_pid, SETPOINT, (float)side_diff, (float)get_curr_time_us())/100;

		right_motor_out = straight_control_params.motor_speed + motor_diff/2;
		left_motor_out = straight_control_params.motor_speed - motor_diff/2;

		if(right_motor_out < 0){
			update_motor(RIGHT_MOTOR, CW, -1*right_motor_out);
		}
		else{
			update_motor(RIGHT_MOTOR, CCW, right_motor_out);
		}

		if(left_motor_out < 0){
			update_motor(LEFT_MOTOR, CCW, -1*left_motor_out);
		}
		else{
			update_motor(LEFT_MOTOR, CW, left_motor_out);
		}
	}

}

void control_open() {
	GPIO_enableInt(INPUT_CTRL_SWITCH, GPIO_INT_BOTH_EDGES); // Enable interrupts
}

void control_init(){

	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2);
	GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

	time_keeper_init();
	pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());

	Semaphore_Params_init(&drive_straight_sem_params);
	drive_straight_sem_params.mode = Semaphore_Mode_BINARY;
	drive_straight_sem_handle = Semaphore_create(0, &drive_straight_sem_params, NULL);
}

void set_pid_kp(char* val) {
	char buf[64];
	uint32_t val_int = atoi(val);
	straight_control_params.kp = val_int/10.0;
	pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());
	uint8_t len = sprintf(buf, "Straight PID KP set to: %3f\r\n", straight_control_params.kp);
	bluetooth_transmit(buf, len);
}

void set_pid_ki(char* val) {
	char buf[64];
	uint32_t val_int = atoi(val);
	straight_control_params.ki = val_int/10.0;
	pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());

	uint8_t len = sprintf(buf, "Straight PID KI set to: %3f\r\n", straight_control_params.ki);
	bluetooth_transmit(buf, len);
}

void set_pid_kd(char* val) {
	char buf[64];
	uint32_t val_int = atoi(val);
	straight_control_params.kd = val_int/10.0;
	pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());

	uint8_t len = sprintf(buf, "Straight PID KD set to: %3f\r\n", straight_control_params.kd);
	bluetooth_transmit(buf, len);
}

void set_motor_speed(char* val) {
	char buf[64];
	straight_control_params.motor_speed = atoi(val);
	update_motor(RIGHT_MOTOR, CCW, straight_control_params.motor_speed);
	update_motor(LEFT_MOTOR, CW, straight_control_params.motor_speed);
	uint8_t len = sprintf(buf, "Motors set to: %i\r\n", straight_control_params.motor_speed);
	bluetooth_transmit(buf, len);
}

void drive_straight_resume(void){
	// Resume the task
	Semaphore_post(drive_straight_sem_handle);
}

void ctrlSwitchFxn(void) {
	// Insert Code For Switch Here
	GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_1);
}
