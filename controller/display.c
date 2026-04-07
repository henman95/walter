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
const UWORD bar_center= bar_size/2;
const UWORD bar_width = 10;
const UWORD bar_space = 10;
const UWORD bar0_l    = 10;
const UWORD bar0_r    = bar0_l + bar_width;
const UWORD bar1_s    = bar0_r + bar_space;
const UWORD bar1_e    = bar1_s + bar_width;
const UWORD bar2_s    = bar1_e + bar_space;
const UWORD bar2_e    = bar2_s + bar_width;
const UWORD bar5_r    = LCD_1IN47_WIDTH - 10;
const UWORD bar5_l    = bar5_r - bar_width;
const UWORD bar4_e    = bar5_l - bar_space;
const UWORD bar4_s    = bar4_e - bar_width;
const UWORD bar3_e    = bar4_s - bar_space;
const UWORD bar3_s    = bar3_e - bar_width;
const UWORD center_h  = LCD_1IN47_WIDTH / 2;
const UWORD center_v  = LCD_1IN47_HEIGHT / 2;

const float rpm_scale = (float)bar_size/(MAX_RPM-MIN_RPM);
const float pwm_scale = (float)bar_size/(float)MAX_PWM;

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
    Paint_Clear(BLUE);
    Paint_SetRotate(ROTATE_180);

    float rpm = state_get_rpm();
    float bias = state_get_bias();
    float rpm_l = state_get_rpm_l()+50;
    float rpm_r = state_get_rpm_r()+50;
    int pwm_l = state_get_pwm_l() + 32768;
    int pwm_r = state_get_pwm_r() + 16758;
   
    UWORD center = (float)(bar_bot - bar_center);
    float bar0_v = (((rpm + bias)*rpm_scale*-1)+center);
    UWORD bar0_t = (UWORD)MIN(bar0_v, center);
    UWORD bar0_b = (UWORD)MAX(bar0_v, center);
    UWORD bar1_v = bar_bot - (UWORD)(rpm_l * rpm_scale);
    UWORD bar2_v = bar_bot - (UWORD)(pwm_l * pwm_scale);
    UWORD bar3_v = bar_bot - (UWORD)(pwm_r * pwm_scale);
    UWORD bar4_v = bar_bot - (UWORD)(rpm_r * rpm_scale);
    float bar5_v = (((rpm - bias)*rpm_scale*-1)+center);
    UWORD bar5_t = (UWORD)MIN(bar5_v, center);
    UWORD bar5_b = (UWORD)MAX(bar5_v, center);

    // printf("%3.2f : %3.2f : %3.2f : %d\n", rpm, rpm_bar, scale_rpm, MAX_RPM);
    Paint_DrawLine(bar0_l-3, bar_bot - bar_center, bar0_r+3, bar_bot - bar_center, RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(bar0_l, bar0_t, bar0_r, bar0_b , RED,   DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar1_s, bar1_v, bar1_e, bar_bot, GREEN, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar2_s, bar2_v, bar2_e, bar_bot, BLACK, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar3_s, bar3_v, bar3_e, bar_bot, BLACK, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawRectangle(bar4_s, bar4_v, bar4_e, bar_bot, GREEN, DOT_PIXEL_2X2, DRAW_FILL_FULL);
    Paint_DrawLine(bar5_l-3, bar_bot - bar_center, bar5_r+3, bar_bot - bar_center, RED, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(bar5_l, bar5_t, bar5_r, bar5_b , RED,   DOT_PIXEL_2X2, DRAW_FILL_FULL);

    if (watchdog_state) {
      Paint_DrawCircle(center_h, 16, 8, RED , DOT_PIXEL_2X2, DRAW_FILL_FULL); 
    } else {
      Paint_DrawCircle(center_h, 16, 8, BLUE, DOT_PIXEL_2X2, DRAW_FILL_FULL); 
    }

    sprintf(buffer, "R: %+3.2f", rpm);
    Paint_DrawString_EN(center_h-30, 30, buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "B: %+3.2f", bias);
    Paint_DrawString_EN(center_h-30, 45, buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "vt: %d", bar0_t);
    Paint_DrawString_EN(center_h-30, 60, buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "vb: %d", bar0_b);
    Paint_DrawString_EN(center_h-30, 75, buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "y : %3.2f", MAX(bar0_v,center));
    Paint_DrawString_EN(center_h-30, 90, buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "y : %3.2f", MIN(bar0_v,center));
    Paint_DrawString_EN(center_h-30, 105, buffer, &Font20, WHITE, BLACK);

    LCD_1IN47_Display(Image);

  }
}
