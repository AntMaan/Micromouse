/*
 * control.h
 *
 *  Created on: Mar 21, 2014
 *      Author: Anthony
 */

#ifndef CONTROL_H_
#define CONTROL_H_

#define SETPOINT 0

#define KP 20
#define KI 2
#define KD 2

#define SPEED 200

void drive_straight();
void control_init();

#endif /* CONTROL_H_ */
