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
#include <string.h>
#include <math.h>

#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/knl/Task.h>

#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>

#include <ti/drivers/GPIO.h>
#include <inc/hw_ints.h>
#include <inc/hw_gpio.h>

#include <xdc/runtime/System.h>

#include "system.h"

#include "drivers/motor.h"
#include "drivers/bluetooth.h"
#include "drivers/ir_sensor.h"
#include "drivers/led.h"

#include "services/pid.h"
#include "services/time_keeper.h"
#include "services/maze_solver.h"

#include "control.h"

#define LPF_SIZE 2

typedef struct straight_pid_params_t {
	float kp;
	float ki;
	float kd;
	uint32_t motor_speed;
}straight_pid_params_t;

typedef struct  {
	float theta;
	float x;
	float y;
	float deltaX;
	float deltaY;
	float deltaTheta;
} dead_reckoning_t;

dead_reckoning_t encoder_estimation = {0, 0, 0, 0, 0, 0};

straight_pid_params_t straight_control_params = {1.0, 1.0, 1.0, 200};


//const float distancePerCount = PI*(2*(float)WHEEL_RADIUS/(float)NUM_TICKS_PER_REVOLUTION);
//const float radiansPerCount = PI*(2*(float)WHEEL_RADIUS/(float)WHEEL_BASE) / (float)NUM_TICKS_PER_REVOLUTION;

Semaphore_Handle drive_straight_sem_handle;
Semaphore_Params drive_straight_sem_params;

walls_t walls;

uint8_t explore = 1;
uint8_t first_check = 1;

side_ir_data_t side_data;

int * path_moves;

control_state_t micromouse_state = RESET;

pid_controller_t side_pid;

bool stream_buf_encoder = false;
char spf_buf_encoder[80];

bool stream_buf_motor = false;
char spf_buf_motor[80];

bool stream_buf_ctl = false;
char spf_buf_ctl[80];

bool delay_on = false;

bool stop_control_loop = true;

uint8_t transition_region = 0;

uint8_t turn_around_dir = 0;
uint8_t turn_around_cal_flag = 1;

#define CENTER_TRANSITION_REGION ((encoders.blocks * NUMTICKS_PER_BLOCK) - HALF_BLOCK)

void control_loop(){


	//uint32_t left_avg;
	//uint32_t right_avg;

	int32_t ir_diff;
	int32_t encoder_diff;


	float measurement;

	int16_t right_motor_out;
	int16_t left_motor_out;
	float error;
	float error_buf[LPF_SIZE];
	float *buf_ptr;
	float *buf_head;
	float *buf_tail;
	char buf[64];
	int str_len;

	buf_head = &error_buf[0];
	buf_ptr=buf_head;
	buf_tail = &error_buf[LPF_SIZE-1];


	while(1){

		Semaphore_pend(drive_straight_sem_handle, BIOS_WAIT_FOREVER);

		check_distance();

		side_poll(&side_data);
		check_walls(&walls, &side_data);

		switch(micromouse_state){

			case STRAIGHT:

				dead_reckoning_update();

//				str_len = sprintf(buf, "X: %3f, Y: %3f, A: %3f\r\n", encoder_estimation.x, encoder_estimation.y, encoder_estimation.theta);
//				bluetooth_transmit(buf, str_len);

//				Task_sleep(1);
//
//				str_len = sprintf(buf, "dX: %3f, dY: %3f, dA: %3f\r\n",  encoder_estimation.deltaX, encoder_estimation.deltaY, encoder_estimation.deltaTheta);
//				bluetooth_transmit(buf, str_len);

//				*buf_ptr = pid_step(&side_pid, SETPOINT, walls.wall_diff, (float)get_curr_time_us())/100.0;
//				buf_ptr = buf_ptr++;
//
//				str_len = sprintf(buf, "En: %f\r\n",  *buf_ptr);
//				bluetooth_transmit(buf, str_len);
//
//				if(buf_ptr > buf_tail){
//					buf_ptr = buf_head;
//				}
//
//				error = 0;
//				uint8_t i;
//				for(i = 0; i < LPF_SIZE; i++){
//					error += error_buf[i];
//				}
//
//				error /= (float)LPF_SIZE;

				if( (!walls.flags.right && !walls.flags.left ) || (transition_region) ){
					error = 0;
				}
				else{
					error = pid_step(&side_pid, SETPOINT, walls.wall_diff, (float)get_curr_time_us())/100.0;
				}

				str_len = sprintf(buf, "E: %f\r\n",  error);
				bluetooth_transmit(buf, str_len);

				// Use the error term to correct the motors
				right_motor_out = straight_control_params.motor_speed - MOTOR_SPEED_OFFSET + error/2.0;
				left_motor_out = straight_control_params.motor_speed + MOTOR_SPEED_OFFSET - error/2.0;

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

				if(stream_buf_motor){
					int len = sprintf(spf_buf_motor, "L: %i, R: %i, D: %f\r\n", left_motor_out, right_motor_out, measurement);
					bluetooth_transmit(spf_buf_motor, len);
				}

				if(stream_buf_ctl){
					int len = sprintf(spf_buf_ctl, "IR: %i, E: %i, M: %f\r\n", ir_diff, encoder_diff, measurement);
					bluetooth_transmit(spf_buf_ctl, len);
				}

				break;

			case TURN_AROUND:

//				if(turn_around_dir){
					update_motor(RIGHT_MOTOR, CW, TURN_SPEED - MOTOR_SPEED_OFFSET);
					update_motor(LEFT_MOTOR, CW, TURN_SPEED + MOTOR_SPEED_OFFSET);
//				}
//				else{
//					update_motor(RIGHT_MOTOR, CCW, TURN_SPEED - MOTOR_SPEED_OFFSET);
//					update_motor(LEFT_MOTOR, CCW, TURN_SPEED + MOTOR_SPEED_OFFSET);
//				}

				pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());
				break;

			case TURN_RIGHT:

				update_motor(RIGHT_MOTOR, CW, TURN_SPEED - MOTOR_SPEED_OFFSET);
				update_motor(LEFT_MOTOR, CW, TURN_SPEED + MOTOR_SPEED_OFFSET);
				pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());
				break;

			case TURN_LEFT:

				update_motor(RIGHT_MOTOR, CCW, TURN_SPEED - MOTOR_SPEED_OFFSET);
				update_motor(LEFT_MOTOR, CCW, TURN_SPEED + MOTOR_SPEED_OFFSET);
				pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());
				break;

			case RESET:

				update_motor(LEFT_MOTOR, CW, 0);
				update_motor(RIGHT_MOTOR, CCW, 0);
				pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());
				break;

			case START:

				Task_sleep(2000);

				walls.wall_int = 0;
				walls.flags.right = 1;
				walls.flags.left = 1;

				if(explore){

					maze_clear();
					maze_update_node(INITIAL_WALLS);

					micromouse_state = (control_state_t) maze_next_direction_dfs();
				}
				else{

					path_moves = maze_dijkstras_algorithm(0, 0, 3, 3);

					micromouse_state = (control_state_t)*path_moves;
					path_moves++;

//					uint8_t i;
//					for(i = 0; i < 16; i++){
						str_len = sprintf(buf, "M: %i\r\n", micromouse_state);
						bluetooth_transmit(buf, str_len);
//						path_moves++;
//					}

				}

				encoders.blocks = 1;
				encoders.left = 0;
				encoders.right = 0;

				break;

			default:
				break;
		}
	}
}

void check_distance(){

	uint32_t avg_ticks;
	avg_ticks = (encoders.right + encoders.left)/2;

	switch(micromouse_state){

		case STRAIGHT:

			if( (avg_ticks >  CENTER_TRANSITION_REGION - TRANSITION_THRESHOLD) && (avg_ticks <  CENTER_TRANSITION_REGION + TRANSITION_THRESHOLD ) ){
				transition_region = 1;
			}
			else{
				transition_region = 0;
			}

			if( (avg_ticks >= encoders.blocks * NUMTICKS_PER_BLOCK) || (walls.flags.front && (avg_ticks >= encoders.blocks * NUMTICKS_PER_BLOCK - 60)) ){

				if(walls.flags.front){
					calibrate_front();
				}

				side_poll(&side_data);
				check_walls(&walls, &side_data);

				if(explore){

					maze_update_node( walls.wall_int );

					micromouse_state = (control_state_t) maze_next_direction_dfs();

				}
				else
				{
					micromouse_state = (control_state_t)*path_moves;
					path_moves++;
				}

				if(micromouse_state != STRAIGHT){
					update_motor(LEFT_MOTOR, BRAKE, 500);
					update_motor(RIGHT_MOTOR, BRAKE, 500);
					Task_sleep(250);
				}

				if(delay_on){
					update_motor(LEFT_MOTOR, CW, 0);
					update_motor(RIGHT_MOTOR, CCW, 0);

					Task_sleep(1000);
				}

				// If we are about to turn left AND we have a wall on our right, calibrate against the right wall
				if(micromouse_state == TURN_LEFT && walls.flags.right){
					calibrate_right();
				}

				// If we are about to turn right AND we have a wall on our left, calibrate against the left wall
				if(micromouse_state == TURN_RIGHT && walls.flags.left){
					calibrate_left();
				}

				// If we are about to turn around AND we have a wall on our right, calibrate against the right wall
				if(micromouse_state == TURN_AROUND && walls.flags.right){
					calibrate_right();
					turn_around_dir = 1;
				}

				// If we are about to turn around AND we have a wall on our left, calibrate against the left wall
				if(micromouse_state == TURN_AROUND && walls.flags.left){
					calibrate_left();
					turn_around_dir = 0;
				}

				if(micromouse_state == STRAIGHT){
					encoders.blocks++;
				}
				else{
					encoders.blocks = 1;
					encoders.right = 0;
					encoders.left = 0;
				}

				if(micromouse_state == RESET){
					explore = 0;
					calibrate_front();
				}

			}

			break;

		case TURN_AROUND:
//
//			if(avg_ticks >= NUMTICKS_HALF_TURN && turn_around_cal_flag){
//
//				// Brake!
//				update_motor(LEFT_MOTOR, BRAKE, 500);
//				update_motor(RIGHT_MOTOR, BRAKE, 500);
//
//				Task_sleep(250);
//
//				if(walls.flags.right){
//					calibrate_right();
//				}
//				if(walls.flags.left){
//					calibrate_left();
//				}
//
//				uint8_t i;
//				for(i = 0; i < 5; i++)
//				{
//				side_poll(&side_data);
//				check_walls(&walls, &side_data);
//				}
//
//				if(walls.flags.front){
//					calibrate_front();
//				}
//
//				turn_around_cal_flag = 0;
//
//			}

			if(avg_ticks >= NUMTICKS_FULL_TURN){

				// Brake!
				update_motor(LEFT_MOTOR, BRAKE, 500);
				update_motor(RIGHT_MOTOR, BRAKE, 500);

				Task_sleep(250);

				uint8_t i;
				for(i = 0; i < 5; i++)
				{
				side_poll(&side_data);
				check_walls(&walls, &side_data);
				}

				if(walls.flags.left && walls.flags.right){
					calibrate_left();
					calibrate_right();
				}

				if(walls.flags.left && !walls.flags.right){
					calibrate_left();
				}

				if(walls.flags.right && !walls.flags.left){
					calibrate_right();
				}

				micromouse_state = STRAIGHT;
				encoders.right = 0;
				encoders.left = 0;

				encoder_estimation.theta = 0;
				encoder_estimation.x = 0;
				encoder_estimation.y = 0;

				turn_around_cal_flag = 1;

			}
			break;

		case TURN_RIGHT:

			if(avg_ticks >= NUMTICKS_HALF_TURN){

				// Brake!
				update_motor(LEFT_MOTOR, BRAKE, 500);
				update_motor(RIGHT_MOTOR, BRAKE, 500);

				Task_sleep(250);

				uint8_t i;
				for(i = 0; i < 5; i++)
				{
				side_poll(&side_data);
				check_walls(&walls, &side_data);
				}

				if(walls.flags.left && walls.flags.right){
					calibrate_left();
					calibrate_right();
				}

				if(walls.flags.left && !walls.flags.right){
					calibrate_left();
				}

				if(walls.flags.right && !walls.flags.left){
					calibrate_right();
				}

				micromouse_state = STRAIGHT;
				encoders.right = 0;
				encoders.left = 0;

				encoder_estimation.theta = 0;
				encoder_estimation.x = 0;
				encoder_estimation.y = 0;

			}
		break;

		case TURN_LEFT:

			if(avg_ticks >= NUMTICKS_HALF_TURN){

				// Brake!
				update_motor(LEFT_MOTOR, BRAKE, 500);
				update_motor(RIGHT_MOTOR, BRAKE, 500);

				Task_sleep(250);

				uint8_t i;
				for(i = 0; i < 5; i++)
				{
				side_poll(&side_data);
				check_walls(&walls, &side_data);
				}

				if(walls.flags.left && walls.flags.right){
					calibrate_left();
					calibrate_right();
				}

				if(walls.flags.left && !walls.flags.right){
					calibrate_left();
				}

				if(walls.flags.right && !walls.flags.left){
					calibrate_right();
				}

				micromouse_state = STRAIGHT;
				encoders.right = 0;
				encoders.left = 0;

				encoder_estimation.theta = 0;
				encoder_estimation.x = 0;
				encoder_estimation.y = 0;

			}
		break;

		default:
			break;
	}

	if(stream_buf_encoder){
		int len = sprintf(spf_buf_encoder, "L: %i, R: %i\r\n", encoders.left , encoders.right );
		bluetooth_transmit(spf_buf_encoder, len);
	}
}
uint16_t previousLeft=0;
uint16_t previousRight=0;

void dead_reckoning_reset(void){
	previousLeft=0;
	previousRight=0;
}


void dead_reckoning_update(void){

	if(encoders.left - previousLeft  < 0 )
		previousLeft=0;

	if(encoders.right - previousRight<0)
		previousRight=0;


	float deltaLeft = encoders.left - previousLeft;
	float deltaRight = encoders.right - previousRight;

//	float deltaDistance = 0.5*(float)(deltaLeft+deltaRight)*distancePerCount;
//
//	encoder_estimation.deltaTheta = (deltaRight-deltaLeft)*radiansPerCount;
//	encoder_estimation.theta += encoder_estimation.deltaTheta;
//
//	encoder_estimation.deltaX = deltaDistance * (float)cos(encoder_estimation.theta);
//	encoder_estimation.deltaY = deltaDistance * (float)sin(encoder_estimation.theta);
//
//	encoder_estimation.x += encoder_estimation.deltaX;//WHEEL_RADIUS * cos(encoder_estimation.theta) * (encoders.right + encoders.left) * (PI/NUM_TICKS_PER_REVOLUTION);
//
//	encoder_estimation.y += encoder_estimation.deltaY;//WHEEL_RADIUS * sin(encoder_estimation.theta) * (encoders.right + encoders.left) * (PI/NUM_TICKS_PER_REVOLUTION);
//
//
	previousLeft=encoders.left;
	previousRight=encoders.right;

	float left_mm = (float) (deltaLeft*MM_PER_TICK);
	float right_mm = (float) (deltaRight*MM_PER_TICK);

	float deltaDistance = (left_mm + right_mm)/2;

	encoder_estimation.deltaTheta = (right_mm - left_mm)/(WHEEL_BASE);
	encoder_estimation.theta += encoder_estimation.deltaTheta;

	encoder_estimation.deltaX = deltaDistance * cos(encoder_estimation.theta);
	encoder_estimation.deltaY = deltaDistance * sin(encoder_estimation.theta);

	encoder_estimation.x += encoder_estimation.deltaX;
	encoder_estimation.y += encoder_estimation.deltaY;

}

void control_open() {
	GPIO_enableInt(INPUT_CTRL_SWITCH, GPIO_INT_BOTH_EDGES); // Enable interrupts
}

void control_init(){

	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_1);
	GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_1, GPIO_STRENGTH_12MA, GPIO_PIN_TYPE_STD);

	time_keeper_init();
	pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());

	Semaphore_Params_init(&drive_straight_sem_params);
	drive_straight_sem_params.mode = Semaphore_Mode_BINARY;
	drive_straight_sem_handle = Semaphore_create(0, &drive_straight_sem_params, NULL);

	micromouse_state = RESET;

	encoder_estimation.theta = 0;
	encoder_estimation.x = 0;
	encoder_estimation.y = 0;
}

void set_pid_kp(char* val) {
	char buf[64];
	uint32_t val_int = atoi(val);
	straight_control_params.kp = val_int/100.0;
	pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());
	uint8_t len = sprintf(buf, "Straight PID KP set to: %3f\r\n", straight_control_params.kp);
	bluetooth_transmit(buf, len);
}

void set_pid_ki(char* val) {
	char buf[64];
	uint32_t val_int = atoi(val);
	straight_control_params.ki = val_int/100.0;
	pid_init(&side_pid, straight_control_params.kp, straight_control_params.ki, straight_control_params.kd, (float)get_curr_time_us());

	uint8_t len = sprintf(buf, "Straight PID KI set to: %3f\r\n", straight_control_params.ki);
	bluetooth_transmit(buf, len);
}

void set_pid_kd(char* val) {
	char buf[64];
	uint32_t val_int = atoi(val);
	straight_control_params.kd = val_int/100.0;
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

void control_loop_resume(void){
	// Resume the task
	Semaphore_post(drive_straight_sem_handle);
}

void ctrlSwitchFxn(void) {

	if (micromouse_state == RESET){
		micromouse_state = START;
	}
	else{
		micromouse_state = RESET;
	}

	GPIOIntClear(GPIO_PORTA_BASE, GPIO_PIN_1);
}

void stream_encoder(char* val) {
	if(strcmp(val, "on") == 0) {
		stream_buf_encoder = true;
	} else {
		if(strcmp(val, "off") == 0) {
			stream_buf_encoder = false;
		} else {
			bluetooth_transmit("Invalid Encoder Stream Value! 'on' or 'off'\r\n", 36);
		}
	}
}

void stream_motor(char* val) {
	if(strcmp(val, "on") == 0) {
		stream_buf_motor = true;
	} else {
		if(strcmp(val, "off") == 0) {
			stream_buf_motor = false;
		} else {
			bluetooth_transmit("Invalid Motor Stream Value! 'on' or 'off'\r\n", 36);
		}
	}
}

void stream_control(char* val) {
	if(strcmp(val, "on") == 0) {
		stream_buf_ctl = true;
	} else {
		if(strcmp(val, "off") == 0) {
			stream_buf_ctl = false;
		} else {
			bluetooth_transmit("Invalid Control Stream Value! 'on' or 'off'\r\n", 36);
		}
	}
}

void move_delay_tog(char* val){
	delay_on ^= 1;
}
