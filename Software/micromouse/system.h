/*
 * system.h
 *
 *  Created on: Mar 18, 2014
 *      Author: Anthony
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_

#define OFF (0)
#define ON (~0)

typedef enum {
	LEFT_MOTOR_DIR_1 = 0,
	LEFT_MOTOR_DIR_2,
	RIGHT_MOTOR_DIR_1,
	RIGHT_MOTOR_DIR_2,
	STBY_MOTOR,

	MICROMOUSE_GPIO_COUNT
} micromouse_gpio_name_t;


/**
 * Enable Flags for System Peripherals, Drivers, and Services
 */

#define MOTORS_ENABLE
#define IR_ADC_SENSORS_ENABLE

void system_init();

#endif /* SYSTEM_H_ */
