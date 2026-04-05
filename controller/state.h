#ifndef _STATE_H_
#define _STATE_H_A

#include "pico/critical_section.h"

typedef struct {
  float  rpm;
  float  bias;
  float  rpm_l;
  float  rpm_r;
  int    pwm_l;
  int    pwm_r;

  bool   display_needed;
} state_t;

extern state_t state;
extern critical_section_t state_lock_token;

void state_init(void);
void state_lock(void);
void state_unlock(void);
void state_test(void);
void task_print_state(void);

void state_display_needed_clear(void);
void state_display_needed_set(void);
bool state_display_needed(void);


// Setters
void state_set_rpm(float setpoint);
void state_set_bias(float setpoint);
void state_set_rpm_l(float setpoint);
void state_set_rpm_r(float setpoint);
void state_set_pwm_l(int pwm);
void state_set_pwm_r(int pwm);

// Getters
float state_get_rpm(void);
float state_get_bias(void);
float state_get_rpm_l(void);
float state_get_rpm_r(void);
int   state_get_pwm_l(void);
int   state_get_pwm_r(void);
#endif
