#ifndef _TASK_MISC_H_
#define _TASK_MISC_H_

#include "pico/stdlib.h"

// Definitions
#define BLINK_LED_PIN 5

void task_blink_led_init(void);
void task_blink_led(void);

#endif
