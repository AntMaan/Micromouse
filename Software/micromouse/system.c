/*
 * sysem.c
 *
 *  Created on: Mar 18, 2014
 *      Author: Anthony Merlino and Brad Ebinger
 */
#include <stdint.h>
#include <stdbool.h>
#include <ti/drivers/GPIO.h>
#include <driverlib/sysctl.h>

#include "system.h"
#include "drivers/motor.h"
#include "drivers/ir_adc_sensor.h"

/* Callback functions for the GPIO interrupt example. */
Void gpioButtonFxn0(Void);
Void gpioButtonFxn1(Void);

/* GPIO configuration structure */
const GPIO_HWAttrs gpioHWAttrs[MICROMOUSE_GPIO_COUNT] = {
	{GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_OUTPUT}, /* LEFT_MOTOR_DIR_1 */
    {GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_OUTPUT}, /* LEFT_MOTOR_DIR_2 */
    {GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_OUTPUT}, /* RIGHT_MOTOR_DIR_1 */
    {GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_OUTPUT}, /* RIGHT_MOTOR_DIR_2 */
    {GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_OUTPUT}, /* STBY_MOTOR */
};

/* Memory for the GPIO module to construct a Hwi */
//Hwi_Struct callbackHwi;

/* GPIO callback structure to set callbacks for GPIO interrupts */
//const GPIO_Callbacks EK_TM4C123GXL_gpioPortFCallbacks = {
//    GPIO_PORTF_BASE, INT_GPIOF, &callbackHwi,
//    {gpioButtonFxn1, NULL, NULL, NULL, gpioButtonFxn0, NULL, NULL, NULL}
//};

const GPIO_Config GPIO_config[] = {
    {&gpioHWAttrs[0]},
    {&gpioHWAttrs[1]},
    {&gpioHWAttrs[2]},
    {&gpioHWAttrs[3]},
    {&gpioHWAttrs[4]},
    {NULL},
};

void system_init(){

	/**
	 * Allow access to all GPIO port registers
	 */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

#ifdef MOTORS_ENABLE
    motors_init();
#endif

#ifdef IR_ADC_SENSORS_ENABLE
    ir_adc_sensor_init();
#endif

    GPIO_init();

}
