#include "motors.h"
#include "hardware/pwm.h"
#include <stdlib.h>
#include <stdio.h>

#include "state.h"

#define PIN_PWMA  6
#define PIN_AIN1  8
#define PIN_AIN2  7
#define PIN_PWMB 26
#define PIN_BIN1  9 
#define PIN_BIN2 25

typedef enum {NEUTRAL,FORWARD,REVERSE,BRAKE} MotorState;

struct MotorChannel {
  uint       gpio_pwm; 
  uint       gpio_in1;
  uint       gpio_in2;

  bool       enabled;
  uint       channel;
  uint       slice_num;
  uint16_t   wrap;
  uint16_t   level;
  float      clkdiv;
  MotorState state;
};


MotorChannel* MotorLeft  = NULL;
MotorChannel* MotorRight = NULL;

MotorChannel* motor_channel_create(uint gpio_pwm, uint gpio_in1, uint gpio_in2) {
  MotorChannel* channel = malloc(sizeof(struct MotorChannel));
  if (channel == NULL) {
    return NULL;
  }

  channel->gpio_pwm  = gpio_pwm;
  channel->gpio_in1  = gpio_in1;
  channel->gpio_in2  = gpio_in2;
  channel->clkdiv    = 4.0f;
  channel->wrap      = 65535;
  channel->level     = 10240;
  channel->channel   = pwm_gpio_to_channel(gpio_pwm);
  channel->slice_num = pwm_gpio_to_slice_num(gpio_pwm);
  channel->enabled   = true;
  channel->state     = NEUTRAL;

  gpio_init(channel->gpio_in1);
  gpio_set_dir(channel->gpio_in1, GPIO_OUT);
  gpio_put(channel->gpio_in1, 0);
  gpio_init(channel->gpio_in2);
  gpio_set_dir(channel->gpio_in2, GPIO_OUT);
  gpio_put(channel->gpio_in2, 0);

  gpio_set_function(gpio_pwm, GPIO_FUNC_PWM);
  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, channel->clkdiv);
  pwm_config_set_wrap(&config, channel->wrap);
  pwm_init(channel->slice_num, &config, true);
  pwm_set_chan_level(channel->slice_num, channel->channel, channel->level);

  return channel;
}

void motor_channel_update(MotorChannel* motor) {
  uint in1,in2;

  if (motor->level > motor->wrap) {
    motor->level = motor->wrap;
  }

  switch (motor->state) {
    case NEUTRAL:
      in1 = 0;
      in2 = 0;
      break;
    case FORWARD:
      in1 = 0;
      in2 = 1;
      break;
    case REVERSE:
      in1 = 1;
      in2 = 0;
      break;
    case BRAKE:
      in1 = 1;
      in2 = 1;
      break;
  }
  uint ino1 = 0;
  uint ino2 = 0;
  printf("%d %d %d %d %d\n", in1,in2,motor->state,ino1,ino2);

  pwm_set_chan_level(motor->slice_num, motor->channel, motor->level);
  gpio_put(motor->gpio_in1, in1);
  gpio_put(motor->gpio_in2, in2);
}

void motors_init() {
  printf("Initializing Motor Controllers\n");
  MotorLeft = motor_channel_create(PIN_PWMA, PIN_AIN1, PIN_AIN2);
  MotorRight = motor_channel_create(PIN_PWMB, PIN_BIN1, PIN_BIN2);
}

void motors_update() {
  float rpm_l = state.rpm - state.bias;
  float rpm_r = state.rpm + state.bias;


  MotorLeft->level  = (uint64_t)(abs(rpm_l) * (65535/190));
  MotorRight->level = (uint64_t)(abs(rpm_r) * (65535/190));
  

  motor_channel_update(MotorLeft);
  motor_channel_update(MotorRight);
}
