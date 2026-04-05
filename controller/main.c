#include "DEV_Config.h"
#include "pico/stdlib.h"

#include "state.h"
#include "display.h"
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
  tasks_add("serial", 50, true, NULL, task_process_serial);
  tasks_add("print_state", 2000, true, NULL, task_print_state);
  tasks_add("display_update", 32, true, display_init, display_update);
  tasks_print();

  tasks_setup();
  tasks_loop();
}
