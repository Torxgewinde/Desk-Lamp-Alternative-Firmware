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

#include <Encoder.h>
#include <OneButton.h>

#define ENCODER_A_PIN 12
#define ENCODER_B_PIN 13
#define ENCODER_SWITCH_PIN 2
#define RESET_BUTTON_PIN 14

Encoder knob(ENCODER_A_PIN, ENCODER_B_PIN);
OneButton button(ENCODER_SWITCH_PIN, true);
OneButton resetSwitch(RESET_BUTTON_PIN, true);

/******************************************************************************
Description.: prepare the rotary knob encoder and switch
Input Value.: -
Return Value: -
******************************************************************************/
void setup_knob() {
  knob.write(0);

  button.attachClick([](){
    Log("Single Click!");

    // change state of desk lamp
    if( state == CONSTANTCOLOR ) {
      state = LIGHTSOFF;
    } else if( state == LIGHTSOFF ) {
      state = CONSTANTCOLOR;

      // come up with a bright light if dimmed very low
      if ( g_WarmWhite + g_ColdWhite == 0 ) {
        g_WarmWhite = 255;
        g_ColdWhite = 0;
      }
    }
  });
  
  button.attachDoubleClick([](){
    Log("Double Click!");
  });
  
  button.attachLongPressStart([](){
    Log("Button long press!");
  });

  resetSwitch.attachLongPressStart([](){
    Log("Reset button was pressed long, resetting configuration");
    state = RESET_CONFIGURATION;
  });
}

/******************************************************************************
Description.: read the rotary knob, change values if required
Input Value.: -
Return Value: -
******************************************************************************/
void loop_knob() {
  long knob_position = knob.read();
  button.tick();
  resetSwitch.tick();

  if(knob_position != 0 && state != LIGHTSOFF) {
    Log("Knob changed by: " + String(knob_position) + ".");

    g_WarmWhite = constrain(knob_position*3 + g_WarmWhite, 0, 255);
    g_ColdWhite = 0;

    knob.write(0);
  }
}
