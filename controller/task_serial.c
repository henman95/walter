#include "task_serial.h"

#include "state.h"
#include "tasks.h"

#include "pico/stdlib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#define CMD_BUFFER_SIZE 80

char cmd_buffer[CMD_BUFFER_SIZE+1];
uint16_t cmd_buffer_index = 0;

typedef struct {
  void (*keyfunc)(void);
  const uint strokes[10];
  uint8_t length;
} stroke_t;

stroke_t strokes[] = {
  {ks_arrow_up,   {27,91,65,0}, 3},
  {ks_arrow_down, {27,91,66,0}, 3},
  {ks_arrow_right, {27,91,67,0}, 3},
  {ks_arrow_left, {27,91,68,0}, 3}
};
const size_t strokes_size = sizeof(strokes) / sizeof(strokes[0]);

bool is_integer(const char *str) {
    if (*str == '\0') return false;
    while (*str != '\0') {
      if (!isdigit(*str)) return false;
      str++;
    }
    return true;
}

bool is_signed_integer(const char *str) {
  if (str[0] == '-'&& is_integer(str+1)) return true;
  return is_integer(str);
}

void ks_arrow_up() {
  float value = state_get_rpm() + 5;
  state_set_rpm(value);
}

void ks_arrow_down() {
  float value = state_get_rpm() - 5;
  state_set_rpm(value);
}

void ks_arrow_left() {
  float value = state_get_bias() - 5;
  state_set_bias(value);
}

void ks_arrow_right() {
  float value = state_get_bias() + 5;
  state_set_bias(value);
}

void serial_init() {
}

void cmd_reset() {
  cmd_buffer_index = 0;
  cmd_buffer[0] = '\0';
}

void serial_update(void) {
  if (cmd_buffer[0]=='\0' && cmd_buffer_index>0) {
    cmd_buffer_index = 0;
  }

  int c = getchar_timeout_us(0);
  if (c != PICO_ERROR_TIMEOUT) {
    // Check for overflow
    if (cmd_buffer_index >= CMD_BUFFER_SIZE+1) {
      printf("Keystroke buffer overflow: MAX=%d\n", CMD_BUFFER_SIZE);
      cmd_reset();
    }
    else if (c=='\b' || c==127) {
      cmd_buffer[cmd_buffer_index--] = '\0';
    }
    else if (c=='`') {
      cmd_reset();
    }
    else {
      cmd_buffer[cmd_buffer_index++] = (char)c;
      cmd_buffer[cmd_buffer_index] = '\0';
    }

    // Check on what to do with buffer if anything
    if (c=='\r' || c=='\n') {
      cmd_buffer[--cmd_buffer_index] = '\0';
      char cmd = cmd_buffer[0];
      char* param = cmd_buffer + 1;
      bool has_param = cmd_buffer[1] != '\0';
      bool has_signed_integer = has_param && is_signed_integer(param);
      bool has_integer = has_param && is_integer(param);
      int value = has_signed_integer ? atoi(param) : 0;

      printf("%c : %c : %b : %b\n", cmd, cmd_buffer[1], has_param, cmd_buffer[1]=='\0');

      if (cmd=='v' && has_signed_integer) {
        state_set_rpm(value);
        printf("Velocity: %d\n", value);
      }
      else if (cmd=='b' && has_signed_integer) {
        state_set_bias(value);
        printf("Bias: %d\n", value);
      }
      else if(cmd=='n' && !has_param) {
        printf("Neutral\n");
        state_set_rpm(0);
        state_set_bias(0);
      }
      else if(cmd=='p' && !has_param) {
        printf("Brake\n");
        state_set_rpm(0);
        state_set_bias(0);
      }
      else if(cmd=='s' && !has_param) {
        printf("Stop\n");
        state_set_rpm(0);
        state_set_bias(0);
      }
      else if(cmd=='d' && !has_param) {
        if (Task_t* task = tasks_find("print_state")) {
            tasks_enable("print_state", !task->enabled);
        }
      }
      else {
        printf("Unknown Command: %s\n", cmd_buffer);
      }
      cmd_reset(); 
    }
    //Check for keystroke patterns
    else {
      for (uint i=0; i<strokes_size; i++ ) {
        bool found = true;
        for (uint x=0; x<strokes[i].length; x++) {
          if (strokes[i].strokes[x] != cmd_buffer[x]) {
            found = false;
          }
        }
        if (found) {
          strokes[i].keyfunc();
          cmd_reset();
          break;
        }
      }
    }
  }
}

