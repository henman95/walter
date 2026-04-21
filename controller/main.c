#include "DEV_Config.h"
#include "pico/stdlib.h"

#include "state.h"
#include "display.h"
#include "motors.h"
#include "task_misc.h"
#include "task_serial.h"
#include "tasks.h"

int main() {
  if (DEV_Module_Init() != 0) {
    return -1;
  }

  printf("Starting\n");
  state_init();
  tasks_init();
  display_init();

  tasks_add("blink_led", 1000, true, task_blink_led_init, task_blink_led);
  tasks_add("watchdog", 1000, true, NULL, display_watchdog_toggle);
  tasks_add("serial", 50, true, serial_init, serial_update);
  //tasks_add("motors", 50, false, motors_init,motors_update);
  tasks_add("print_state", 2000, false, NULL, task_print_state);
  tasks_add("display_update", 32, true, display_init, display_update);
  tasks_print();

  tasks_setup();
  tasks_loop();
}
