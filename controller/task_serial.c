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

bool is_integer(const char *str) {
    if (*str == '\0') return false;
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}

bool is_signed_integer(const char *str) {
  if (str[0] == '-' && is_integer(str+1)) return true;
  return is_integer(str);
}

void process_command(void) {
  cmd_buffer[cmd_buffer_index] = '\0';
  printf("\nReceived Command: %s\n", cmd_buffer);

  uint len = strlen(cmd_buffer);

  // Check for Null string
  if (len == 0){
    printf("Null String\n");
    return;
  }

  char cmd = (char)cmd_buffer[0];

  // Velocity 
  if (cmd == 'v' && len > 1 && is_signed_integer(cmd_buffer+1)) {
      int velocity = atoi(cmd_buffer+1);
      printf("Velocity: %d\n", velocity);
      state_set_rpm(velocity);
  }
  // Bias
  else if (cmd == 'b' && len > 1 && is_signed_integer(cmd_buffer+1)) {
      int bias = atoi(cmd_buffer+1);
      printf("Bias: %d\n", bias);
      state_set_bias(bias);
  }
  // Forward Command
  else if (cmd == 'f' && len > 1 && is_integer(cmd_buffer+1)) {
    int velocity = atoi(cmd_buffer+1);
    printf("Forward: %d\n", velocity);
  }
  // Reverse Command
  else if (cmd == 'r' && len > 1 && is_integer(cmd_buffer+1)) {
    int velocity = atoi(cmd_buffer+1);
    printf("Reverse: %d\n", velocity);
  }
  else if (cmd == 'p' && len == 1) {
    printf("Park\n");
    state_set_rpm(0.0);
  }
  else if (cmd == 'n' && len == 1) {
    printf("Neutral\n");
    state_set_rpm(0.0);
  }
  else if (cmd == 's' && len == 1) {
    printf("Stop\n");
    state_set_rpm(0.0);
  }
  else if (cmd == 'd' && len == 1) {
    printf("Toggle state display\n");
    Task_t* task = tasks_find("print_state");
    if (task != NULL) {
      tasks_enable("print_state", !task->enabled);
    }
  } else {
    printf("Uknown Command: %c\n", cmd);
  }

  cmd_buffer_index = 0;
}

void task_process_serial(void) {
  if (cmd_buffer[0]=='\0' && cmd_buffer_index>0) {
    cmd_buffer_index = 0;
  }

  int c = getchar_timeout_us(0);
  if (c != PICO_ERROR_TIMEOUT) {
    if (c=='\r' || c=='\n') {
      if (cmd_buffer_index > 0) {
        process_command();
      }
    }
    else if (c=='\b' || c==127) {
      if (cmd_buffer_index > 0) {
        cmd_buffer_index--;
      }
    }
    else {
      if (cmd_buffer_index < CMD_BUFFER_SIZE+1) {
        cmd_buffer[cmd_buffer_index++] = (char)c;
      }  
    }
  }
}

