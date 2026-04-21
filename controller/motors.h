#ifndef _MOTORS_H_
#define _MOTORS_H_

#include "hardware/gpio.h"

typedef struct MotorChannel MotorChannel;

extern MotorChannel* Motor1;
extern MotorChannel* Motor2;

MotorChannel* motor_channel_create(uint gpio_pwm, uint gpio_in1, uint gpio_in2);

void motors_init(void);
void motors_update(void);
#endif
