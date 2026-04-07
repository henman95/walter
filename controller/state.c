#include "state.h"

#include "stdio.h"

state_t state;
critical_section_t state_lock_token;

void state_init() {
  state.rpm   = 0;
  state.bias  = 0;
  state.rpm_l = 0;
  state.rpm_r = 0;
  state.pwm_l = 0;
  state.pwm_r = 0;

  state.display_needed = true;

  critical_section_init(&state_lock_token);
}

void state_lock() {
  printf("a0\n");
  critical_section_enter_blocking(&state_lock_token);
  printf("a1\n");
}

void state_unlock() {
  printf("b0\n");
  critical_section_exit(&state_lock_token);
  printf("b1\n");
}

void task_print_state() {
  printf("RPM: %3.2f Bias: %3.2f", state.rpm, state.bias);
  printf(" -- ");
  printf("L: Setpoint:%3.2f Current:%3.2f PWM:%d", state.rpm + state.bias, state.rpm_l, state.pwm_l);
  printf(" -- ");
  printf("R: Setpoint:%3.2f Current:%3.2f PWM:%d", state.rpm - state.bias, state.rpm_r, state.pwm_r);
  printf("\n");
}

void state_display_needed_clear() {
    critical_section_enter_blocking(&state_lock_token);
    state.display_needed = false;
    critical_section_exit(&state_lock_token);
}

void state_display_needed_set() {
    critical_section_enter_blocking(&state_lock_token);
    state.display_needed = true; 
    critical_section_exit(&state_lock_token);
}

bool state_display_needed() {
    bool value;
    critical_section_enter_blocking(&state_lock_token);
    value = state.display_needed;
    critical_section_exit(&state_lock_token);
    return value;
}

// ====================== SETTERS ======================
void state_set_rpm(float rpm)
{
    if (rpm > MAX_RPM) rpm = MAX_RPM;
    if (rpm < MIN_RPM) rpm = MIN_RPM;
    critical_section_enter_blocking(&state_lock_token);
    state.rpm = rpm;
    critical_section_exit(&state_lock_token);
    state_display_needed_set();
}

void state_set_bias(float bias )
{
    if (bias > MAX_BIAS) bias = MAX_BIAS;
    if (bias < MIN_BIAS) bias = MIN_BIAS;
    critical_section_enter_blocking(&state_lock_token);
    state.bias = bias;
    critical_section_exit(&state_lock_token);
    state_display_needed_set();
}

void state_set_rpm_l(float rpm)
{
    critical_section_enter_blocking(&state_lock_token);
    state.rpm_l = rpm;
    critical_section_exit(&state_lock_token);
    state_display_needed_set();
}

void state_set_rpm_r(float rpm)
{
    critical_section_enter_blocking(&state_lock_token);
    state.rpm_r = rpm;
    critical_section_exit(&state_lock_token);
    state_display_needed_set();
}

void state_set_pwm_l(int pwm)
{
    critical_section_enter_blocking(&state_lock_token);
    state.pwm_l = pwm;
    critical_section_exit(&state_lock_token);
    state_display_needed_set();
}

void state_set_pwm_r(int pwm)
{
    critical_section_enter_blocking(&state_lock_token);
    state.pwm_r = pwm;
    critical_section_exit(&state_lock_token);
    state_display_needed_set();
}

// ====================== GETTERS ======================

float state_get_rpm(void)
{
    critical_section_enter_blocking(&state_lock_token);
    float val = state.rpm;
    critical_section_exit(&state_lock_token);
    return val;
}

float state_get_bias(void)
{
    critical_section_enter_blocking(&state_lock_token);
    float val = state.bias;
    critical_section_exit(&state_lock_token);
    return val;
}


float state_get_rpm_l(void)
{
    critical_section_enter_blocking(&state_lock_token);
    float val = state.rpm_r;
    critical_section_exit(&state_lock_token);
    return val;
}

float state_get_rpm_r(void)
{
    critical_section_enter_blocking(&state_lock_token);
    float val = state.rpm_l;
    critical_section_exit(&state_lock_token);
    return val;
}

int state_get_pwm_l(void)
{
    critical_section_enter_blocking(&state_lock_token);
    int val = state.pwm_l;
    critical_section_exit(&state_lock_token);
    return val;
}

int state_get_pwm_r(void)
{
    critical_section_enter_blocking(&state_lock_token);
    int val = state.pwm_r;
    critical_section_exit(&state_lock_token);
    return val;
}


