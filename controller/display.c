#include "display.h"
#include "stdio.h"

#include "state.h"
#include "LCD_1in47.h"
#include "GUI_Paint.h"

UDOUBLE Imagesize = LCD_1IN47_HEIGHT * LCD_1IN47_WIDTH * 2;
UWORD *Image;

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
    Paint_Clear(WHITE);
    Paint_SetRotate(ROTATE_180);

    float rpm = state_get_rpm();
    float bias = state_get_bias();
    float rpm_l = state_get_rpm_l();
    float rpm_r = state_get_rpm_r();
    int pwm_l = state_get_pwm_l();
    int pwm_r = state_get_pwm_r();

    sprintf(buffer, "RPM : %+3.2f", rpm);
    Paint_DrawString_EN(1,20,buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "BIAS: %+3.2f", bias);
    Paint_DrawString_EN(1,35,buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "L: % 3.2f : % 3.2f :%d", rpm + bias, rpm_l, rpm_l);
    Paint_DrawString_EN(1,60,buffer, &Font20, WHITE, BLACK);
    sprintf(buffer, "R: % 3.2f : % 3.2f :%d", rpm - bias, rpm_r, rpm_r);
    Paint_DrawString_EN(1,75,buffer, &Font20, WHITE, BLACK);

    if (watchdog_state) {
      Paint_DrawCircle(300, 16, 8, RED, DOT_PIXEL_2X2, DRAW_FILL_FULL); 
    } else {
      Paint_DrawCircle(300, 16, 8, WHITE, DOT_PIXEL_2X2, DRAW_FILL_FULL); 
    }

    LCD_1IN47_Display(Image);

  }
}
