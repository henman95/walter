#include "display.h"
#include "stdio.h"

#include "state.h"
#include "LCD_1in47.h"
#include "GUI_Paint.h"

UDOUBLE Imagesize = LCD_1IN47_HEIGHT * LCD_1IN47_WIDTH * 2;
UWORD *Image;

// 320x172
const UWORD bar_bot   = LCD_1IN47_HEIGHT - 10;
const UWORD bar_top   = 10;
const UWORD bar_size  = bar_bot - bar_top;
const UWORD bar_width = 10;
const UWORD bar_space = 10;
const UWORD bar0_s    = 10;
const UWORD bar0_e    = bar0_s + bar_width;
const UWORD bar1_s    = bar0_e + bar_space;
const UWORD bar1_e    = bar1_s + bar_width;
const UWORD bar2_s    = bar1_e + bar_space;
const UWORD bar2_e    = bar2_s + bar_width;
const UWORD bar5_e    = LCD_1IN47_WIDTH - 10;
const UWORD bar5_s    = bar5_e - bar_width;
const UWORD bar4_e    = bar5_s - bar_space;
const UWORD bar4_s    = bar4_e - bar_width;
const UWORD bar3_e    = bar4_s - bar_space;
const UWORD bar3_s    = bar3_e - bar_width;
const UWORD center    = LCD_1IN47_HEIGHT / 2;

const float rpm_scale = (float)bar_size/(float)MAX_RPM;
const float pwm_scale = (float)bar_size/(float)65535;

bool watchdog_state = false;

void display_watchdog_toggle() {
  watchdog_state = !watchdog_state;
  state_display_needed_set();
}

void display_init() {
  printf("Display Init\n");
  LCD_1IN47_Init(VERTICAL);
  LCD_1IN47_Clear(WHITE);

  if ((Image = (UWORD *)malloc(Imagesize)) == NULL){
    printf("Failed to alloc LCD memory\n");
  }

}

void display_update() {
  char buffer[100];

  if (state_display_needed()) {
    state_display_needed_clear();

    Paint_NewImage((UBYTE *)Image, LCD_1IN47_WIDTH, LCD_1IN47_HEIGHT, 0, WHITE);
    Paint_SetScale(65);
    Paint_Clear(CYAN);
    Paint_SetRotate(ROTATE_180);

    float rpm = state_get_rpm();
    float bias = state_get_bias();
    float rpm_l = state_get_rpm_l()+50;
    float rpm_r = state_get_rpm_r()+50;
    int pwm_l = state_get_pwm_l() + 32768;
    int pwm_r = state_get_pwm_r() + 16758;

    UWORD bar0_v = bar_bot - (UWORD)((rpm + bias) * rpm_scale);
    UWORD bar1_v = bar_bot - (UWORD)(rpm_l * rpm_scale);
    UWORD bar2_v = bar_bot - (UWORD)(pwm_l * pwm_scale);
    UWORD bar3_v = bar_bot - (UWORD)(pwm_r * pwm_scale);
    UWORD bar4_v = bar_bot - (UWORD)(rpm_r * rpm_scale);
    UWORD bar5_v = bar_bot - (UWORD)((rpm - bias) * rpm_scale);

    // printf("%3.2f : %3.2f : %3.2f : %d\n", rpm, rpm_bar, scale_rpm, MAX_RPM);
    Paint_DrawRectangle(bar0_s, bar0_v, bar0_e, bar_bot, RED, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar1_s, bar1_v, bar1_e, bar_bot, GREEN, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar2_s, bar2_v, bar2_e, bar_bot, BLACK, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar3_s, bar3_v, bar3_e, bar_bot, BLACK, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar4_s, bar4_v, bar4_e, bar_bot, GREEN, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar5_s, bar5_v, bar5_e, bar_bot, RED, DOT_PIXEL_2X2, DRAW_FILL_FULL);

    if (watchdog_state) {
      Paint_DrawCircle(center, 16, 8, RED  , DOT_PIXEL_2X2, DRAW_FILL_FULL); 
    } else {
      Paint_DrawCircle(center, 16, 8, WHITE, DOT_PIXEL_2X2, DRAW_FILL_FULL); 
    }

    sprintf(buffer, "R: %+3.2f", rpm);
    Paint_DrawString_EN(center, 30, buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "B: %+3.2f", bias);
    Paint_DrawString_EN(center, 45, buffer, &Font20, WHITE, BLACK);

    LCD_1IN47_Display(Image);

  }
}
