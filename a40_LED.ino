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
              a gamma correction is usual.
Input Value.: brightness level for one color (0 .. 255)
Return Value: gamma corrected value (0 .. 255)
******************************************************************************/
uint8_t gamma_correction(uint8_t value) {
  //override the gamma function for low values
  if(value < 2)
    return 0;

  // since ESP8266 is reasonably fast, floating point calculations are acceptable
  // the alternative is using a LUT, if running low on CPU cycles.
  return constrain(11 + round(0.00277*value*value + 0.25574*value), 0, 255);
}

/******************************************************************************
Description.: set the LED
Input Value.: brightness level for leds
Return Value: -
******************************************************************************/
void setLeds(uint8_t warmwhite, uint8_t coldwhite) {
  analogWrite(WARMWHITE_PIN, gamma_correction(warmwhite));
  analogWrite(COLDWHITE_PIN, gamma_correction(coldwhite));
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
Description.: set the LEDs, not immediatly but fading from previous color
              to the specified within the specified time
Input Value.: brightness level for LEDs
              ration of coldwhite to warmwhite, eg. 1.0 is warmwhite, 0.0 is cw
              fade_time is the time of animation in milliseconds
Return Value: -
******************************************************************************/
void setLedsAnimatedRB(float ratio, float brightness, uint32_t duration) {
  uint8_t warmwhite, coldwhite;

  warmwhite = 255 * ratio * brightness;
  coldwhite = 255 * (1-ratio) * brightness;

  setLedsAnimated(warmwhite, coldwhite, duration);
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

  setLedsAnimatedRB(g_ratio, g_brightness, 1000);
}

/******************************************************************************
Description.: set the RGBW LEDs as defined by state and global colors variables
Input Value.: -
Return Value: -
******************************************************************************/
void loop_LEDs() {
  uint8_t a;
  
  switch(state) {
    case BOOTUP:
      setLedsAnimatedRB(g_ratio, g_brightness, 0);
      break;
      
    case CONSTANTCOLOR:
      setLedsAnimatedRB(g_ratio, g_brightness, 500);
      break;

    case LIGHTSOFF:
      setLedsAnimated(0, 0, 2000);
      break;

    case RESET_CONFIGURATION:
      // hectic animation during reset, since millis only changes every millisecond
      // this animation will not depend very much on how often this loop-function is executed
      a = millis() % 256;
      setLeds(a, 255-a);
      break;

    default:
      ;
  }
}
