#include "task_serial.h"

#include "motors.h"
#include "state.h"
#include "tasks.h"

#include "ctype.h"
#include "pico/stdlib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define CMD_BUFFER_SIZE 80

char cmd_buffer[CMD_BUFFER_SIZE + 1];
uint16_t cmd_buffer_index = 0;

bool is_param_integer(const char *str) {
  if (*str == '\0')
    return false;
  if (*str == '-')
    str++;

  while (*str != '\0') {
    if (!isdigit(*str))
      return false;
    str++;
  }
  return true;
}

void serial_init() {}

void cmd_reset() {
  cmd_buffer_index = 0;
  cmd_buffer[0] = '\0';
}

void serial_update(void) {
  if (cmd_buffer[0] == '\0' && cmd_buffer_index > 0) {
    cmd_buffer_index = 0;
  }

  int c = getchar_timeout_us(0);
  if (c != PICO_ERROR_TIMEOUT) {
    // Check for overflow
    if (cmd_buffer_index >= CMD_BUFFER_SIZE + 1) {
      printf("Keystroke buffer overflow: MAX=%d\n", CMD_BUFFER_SIZE);
      cmd_reset();
    } else if (c == '\b' || c == 127) {
      cmd_buffer[cmd_buffer_index--] = '\0';
    } else if (c == '`') {
      printf("Reset\n");
      cmd_reset();
    } else if (c == '\t') {
      printf("Buffer:%s:\n", cmd_buffer);
    }

    else if (c == '\r' || c == '\n') {
      uint cmd_len = strlen(cmd_buffer);
      char cmd = cmd_buffer[0];
      char *param = cmd_buffer + 1;
      bool has_param = *param != '\0';
      bool has_integer = is_param_integer(param);
      int value = has_integer ? atoi(param) : 0;

      switch (cmd) {
      case 'v':
        if (has_integer)
          state_set_rpm(value);
        break;
      case 'b':
        if (has_integer)
          state_set_bias(value);
        break;
      case 'n':
        if (!has_param) {
          state_set_rpm(0);
          state_set_bias(0);
        }
        break;
      case 'i':
        if (!has_param) 
          motors_init();
        break;
      case 'u':
        if (!has_param)
          motors_update();
        break;
      case 'p':
        if (!has_param)
          motors_print();
        break;
      default:
        printf("Unknown Command: %c\n", cmd);
      }
      cmd_reset();
    } else {
      cmd_buffer[cmd_buffer_index++] = (char)c;
      cmd_buffer[cmd_buffer_index] = '\0';
      cmd_buffer[cmd_buffer_index + 1] = '\0';
    }
  }
}
