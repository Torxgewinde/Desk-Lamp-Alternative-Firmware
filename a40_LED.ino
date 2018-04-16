/*******************************************************************************
#                                                                              #
#     An alternative firmware for Xiaomi Desk Lamp (Yeelight)                  #
#                                                                              #
#                                                                              #
#      Copyright (C) 2018 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
********************************************************************************/

#include <Ticker.h>

#define WARMWHITE_PIN 5
#define COLDWHITE_PIN 4

// how often will the animation ticker call the callback, defined in ms
#define ANI_RES 10

/******************************************************************************
Description.: The visual impression of the LEDs is not linear like the PWM,
              thus to make the range match the visual impression better
              a gamma correction is usual. Here, just one color channel
              is adjusted, which fixes the most important issues. Dependencies
              of one color with the others is not improved this way, so color
              shifts in mixed colors are still possible.
              Credit for the LUT values goes to
              http://rgb-123.com/ws2812-color-output/
Input Value.: brightness level for one color (0 .. 255)
Return Value: gamma corrected value (0 .. 255)
******************************************************************************/
uint8_t gamma_correction(uint8_t value) {
  uint8_t gammaE[] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5,
    6, 6, 6, 7, 7, 7, 8, 8, 8, 9, 9, 9, 10, 10, 11, 11,
    11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
    19, 19, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 27, 28,
    29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37, 37, 38, 39, 40,
    40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
    71, 72, 73, 74, 76, 77, 78, 79, 80, 81, 83, 84, 85, 86, 88, 89,
    90, 91, 93, 94, 95, 96, 98, 99,100,102,103,104,106,107,109,110,
    111,113,114,116,117,119,120,121,123,124,126,128,129,131,132,134,
    135,137,138,140,142,143,145,146,148,150,151,153,155,157,158,160,
    162,163,165,167,169,170,172,174,176,178,179,181,183,185,187,189,
    191,193,194,196,198,200,202,204,206,208,210,212,214,216,218,220,
    222,224,227,229,231,233,235,237,239,241,244,246,248,250,252,255};
  
  return gammaE[value];
}

/******************************************************************************
Description.: set the LED, operating all LEDs at the same time exceeds the power
              supply limit
Input Value.: brightness level for leds
Return Value: -
******************************************************************************/
void setLeds(uint8_t warmwhite, uint8_t coldwhite) {
  int excess = max(0, warmwhite+coldwhite-255);
  int ww = constrain(warmwhite - excess/2, 0, 255);
  int cw = constrain(coldwhite - excess/2, 0, 255);
  
  analogWrite(WARMWHITE_PIN, gamma_correction(ww));
  analogWrite(COLDWHITE_PIN, gamma_correction(cw));
}

/******************************************************************************
Description.: animation function performs a fade animation
Input Value.: a pointer to context variables required for the animation
Return Value: -
******************************************************************************/
struct _animation_context {
  uint8_t next_ww;
  uint8_t next_cw;

  uint8_t previous_ww;
  uint8_t previous_cw;

  uint8_t current_ww;
  uint8_t current_cw;

  bool busy;
  uint32_t ani_time;
  uint32_t ani_duration;

  Ticker animationTicker;
} animation_context;

void animation(void *vctx) {
  float progress;
  _animation_context *ctx = (_animation_context *)vctx;

  ctx->ani_time += ANI_RES;
  progress = _min(1.0, (float)ctx->ani_time / (float)ctx->ani_duration);

  ctx->current_cw = ctx->previous_cw + (ctx->next_cw - ctx->previous_cw) * progress;
  ctx->current_ww = ctx->previous_ww + (ctx->next_ww - ctx->previous_ww) * progress;

  setLeds(ctx->current_ww, ctx->current_cw);

  if( progress >= 1.0 ) {
    // stop timer, we reached 100%
    ctx->animationTicker.detach();
    ctx->ani_time = 0;
    ctx->previous_ww = ctx->next_ww;
    ctx->previous_cw = ctx->next_cw;
    ctx->busy = false;
  }
}

/******************************************************************************
Description.: set the LEDs, not immediatly but fading from previous color
              to the specified within the specified time
Input Value.: brightness level for LEDs
              fade_time is the time of animation in milliseconds
Return Value: -
******************************************************************************/
void setLedsAnimated(uint8_t warmwhite, uint8_t coldwhite, uint32_t duration) {

  // guard against restarting the animation unless new target color specified
  if( (warmwhite == animation_context.next_ww) && (coldwhite == animation_context.next_cw) ) {
    // leave if we already currently animate towards or already have set desired color
    return;
  }

  // if animation is still running
  if(animation_context.busy) {
    // already/still busy: stop timer first
    animation_context.animationTicker.detach();

    // restart animation from current colors
    animation_context.previous_ww = animation_context.current_ww;
    animation_context.previous_cw = animation_context.current_cw;
  }

  animation_context.ani_duration = duration;
  animation_context.ani_time = 0;
  animation_context.busy = true;

  // animate towards this color
  animation_context.next_ww = warmwhite;
  animation_context.next_cw = coldwhite;

  animation_context.animationTicker.attach_ms(ANI_RES, animation, (void *)&animation_context);
}

/******************************************************************************
Description.: prepare the LEDs, switch on to show lamp is working and provide
              light
Input Value.: -
Return Value: -
******************************************************************************/
void setup_LEDs() {
  pinMode(WARMWHITE_PIN, OUTPUT);
  pinMode(COLDWHITE_PIN, OUTPUT);
  
  analogWriteFreq(500);
  analogWriteRange(255);

  animation_context.next_ww = 0;
  animation_context.next_cw = 0;
  
  animation_context.previous_ww = 0;
  animation_context.previous_cw = 0;
  
  animation_context.current_ww = 0;
  animation_context.current_cw = 0;

  animation_context.busy = false;

  setLedsAnimated(g_WarmWhite, g_ColdWhite, 1000);
}

/******************************************************************************
Description.: set the RGBW LEDs as defined by state and global colors variables
Input Value.: -
Return Value: -
******************************************************************************/
void loop_LEDs() {  
  switch(state) {
    case BOOTUP:
      setLeds(g_WarmWhite, g_ColdWhite);
      break;
      
    case CONSTANTCOLOR:
      setLedsAnimated(g_WarmWhite, g_ColdWhite, 1000);
      break;

    case LIGHTSOFF:
      setLedsAnimated(0, 0, 2000);
      break;

    case RESET_CONFIGURATION:
      // hectic animation during reset, since millis only changes every millisecond
      // this animation will not depend very much on how often this loop-function is executed
      g_ColdWhite = millis() % 256;
      g_WarmWhite = 255 - g_ColdWhite;
      setLeds(g_WarmWhite, g_ColdWhite);
      break;

    default:
      ;
  }
}
