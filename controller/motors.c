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

//typedef enum {NEUTRAL,FORWARD,REVERSE,BRAKE} MotorState;

typedef struct {
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
} MotorChannel_t;


static MotorChannel_t motor_left  = {0};
static MotorChannel_t motor_right = {0};

void motor_channel_init(MotorChannel_t* channel, uint gpio_pwm, uint gpio_in1, uint gpio_in2) {
  /*
  MotorChannel* channel = malloc(sizeof(struct MotorChannel));
  if (channel == NULL) {
    return NULL;
  }
  */

  printf("  enabling Motor on %u\n", gpio_pwm);

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

  printf("  pin   %u\n", channel->gpio_pwm);
  printf("  level %u\n", channel->level);
  //return channel;
}

void motor_channel_update(MotorChannel_t* motor) {
  //printf("Motor - %u %u %u\n",motor->gpio_pwm, motor->gpio_in1, motor->gpio_in2);
  if (motor->level > motor->wrap) {
    motor->level = motor->wrap;
  }
  //printf("  Level - %u\n", motor->level);

  switch (motor->state) {
    case NEUTRAL:
      gpio_put(motor->gpio_in1, 0);
      gpio_put(motor->gpio_in2, 0);
      break;
    case FORWARD:
      gpio_put(motor->gpio_in1, 0);
      gpio_put(motor->gpio_in2, 1);
      break;
    case REVERSE:
      gpio_put(motor->gpio_in1, 1);
      gpio_put(motor->gpio_in2, 0);
      break;
    case BRAKE:
      gpio_put(motor->gpio_in1, 1);
      gpio_put(motor->gpio_in2, 1);
      break;
  }
  //printf("Slice: %u Channel:%u State:%d Level:%d\n", motor->slice_num, motor->channel, motor->state, motor->level);
  pwm_set_chan_level(motor->slice_num, motor->channel, motor->level);
}

void motors_init() {
  printf("Initializing Motor Controllers\n");
  motor_channel_init(&motor_left , PIN_PWMA, PIN_AIN1, PIN_AIN2);
  motor_channel_init(&motor_right, PIN_PWMB, PIN_BIN1, PIN_BIN2);
}

void motors_update() {
  float rpm_l = state.rpm - state.bias;
  float rpm_r = state.rpm + state.bias;

  motor_left.level  = (uint64_t)(abs(rpm_l) * (65535/190));
  motor_right.level = (uint64_t)(abs(rpm_r) * (65535/190));
  state_set_pwm_l(motor_left.level);
  state_set_pwm_r(motor_right.level);

  if (rpm_l > 0) {
    motor_left.state = FORWARD;
  } else if (rpm_l < 0) {
    motor_left.state = REVERSE;
  } else {
    motor_left.state = NEUTRAL;
  }
  state_set_state_l(motor_left.state);

  if (rpm_r > 0) {
    motor_right.state = FORWARD;
  } else if (rpm_r < 0) {
    motor_right.state = REVERSE;
  } else {
    motor_right.state = NEUTRAL;
  }
  state_set_state_r(motor_right.state);  
  //printf(" ML- %u %u\n", motor_left.gpio_pwm, motor_left.level);
  //printf(" MR- %u %u\n", motor_right.gpio_pwm, motor_right.level);

  motor_channel_update(&motor_left);
  motor_channel_update(&motor_right);
}

void motors_print() {
  printf(" Left: pin:%u  state:%d  level:%d\n", motor_left.gpio_pwm, motor_left.state, motor_left.level);
  printf("Right: pin:%u  state:%d  level:%d\n", motor_right.gpio_pwm, motor_right.state, motor_right.level);
}

