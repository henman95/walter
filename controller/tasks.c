#include "tasks.h"

#include "stdio.h"
#include "string.h"

Task_t tasks[MAX_TASKS];
uint8_t task_count = 0;

void tasks_init() {
  task_count = 0;
  for (uint8_t i=0; i<MAX_TASKS; i++) {
    tasks[i].enabled = false;
  }
}

bool tasks_add(const char* name, uint32_t interval_ms, bool enabled, void (*setup)(void), void (*run)(void)) {
  if (task_count >= MAX_TASKS) {
    printf("Error: Maximum number of tasks (%d) reached.\n", MAX_TASKS);
    return false;
  }

  if (setup == NULL && run == NULL) {
    printf("Error: Both setup and run function cannot be NULL.\n");
    return false;
  }

  tasks[task_count].name = name;
  tasks[task_count].interval_us = (uint64_t)interval_ms * 1000ULL;
  tasks[task_count].enabled = enabled;
  tasks[task_count].lastrun_us = 0;
  tasks[task_count].setup = setup;
  tasks[task_count].run = run;

  printf("Added Task: %s\n", name);
  task_count++;
  return true;
}

void tasks_enable(const char* name, bool enabled) {
  for (uint8_t i=0;i<task_count; i++) {
    if (strcmp(tasks[i].name,name) == 0) {
      tasks[i].enabled = enabled;
    }
  }
}

Task_t* tasks_find(const char* name) {
  for (uint8_t i=0;i<task_count; i++) {
    if (strncmp(tasks[i].name,name,40) == 0) {
      return &tasks[i];
    }
  }
  return NULL;
}

void tasks_setup() {
  for (uint8_t i=0; i<task_count; i++) {
    if (tasks[i].enabled && tasks[i].setup != NULL) {
      tasks[i].setup();
    }
  }
}

void tasks_run() {
  uint64_t now_us = time_us_64();

  for (uint8_t i=0; i<task_count; i++) {
    uint64_t elapsed = now_us - tasks[i].lastrun_us;
    if (tasks[i].enabled && elapsed >= tasks[i].interval_us) {
      tasks[i].run();
      tasks[i].lastrun_us = now_us;
    }
  }
}

void tasks_loop(){
  printf("Starting Loop\n");
  while (true) {
    tasks_run();
  }
}

void tasks_print() {
  for (uint8_t i=0; i<task_count; i++) {
    if (tasks[i].enabled) {
      printf("%s %d %b\n", tasks[i].name, tasks[i].interval_us/1000, tasks[i].enabled);
    }
  }
}
