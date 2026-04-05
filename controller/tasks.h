#ifndef _TASK_H_
#define _TASK_H_

#include <pico/stdlib.h>

typedef struct {
  const char* name;
  uint64_t interval_us;
  uint64_t lastrun_us;
  bool enabled;
  void (*setup)(void);
  void (*run)(void);
} Task_t;

#define MAX_TASKS 15

void tasks_init(void);
bool tasks_add(const char* name, uint32_t interval_ms, bool enabled, void (*setup)(void), void (*run)(void));
void tasks_enable(const char* name, bool);
Task_t* tasks_find(const char*);
void tasks_loop(void);
void tasks_run(void);
void tasks_print(void);
void tasks_setup(void);

#endif
