#include "task_misc.h"

#include "stdio.h"

void task_blink_led_init(void) {
  gpio_init(BLINK_LED_PIN);
  gpio_set_dir(BLINK_LED_PIN, GPIO_OUT);
}

void task_blink_led(void) {
  gpio_xor_mask(1u << BLINK_LED_PIN);
}

