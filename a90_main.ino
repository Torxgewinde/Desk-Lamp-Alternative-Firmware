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

/******************************************************************************
Description.: prepares everything, sets the light up first and then does the
              more time consuming tasks
Input Value.: -
Return Value: -
******************************************************************************/
void setup(void){
  state = BOOTUP;

  Serial.begin(115200);
  Serial.println("XIAOMI Desk Lamp starting up\nCompiled at: " __DATE__ " - " __TIME__);

  log_messages.resize(LOG_LENGTH, "-");
  Log("XIAOMI Desk Lamp starting up");
  Log("Compiled at: " __DATE__ " - " __TIME__);
  
  Log("initializing Filesystem");
  if( !LittleFS.begin() ) {
    Log("Filesystem not mounted correctly, retrying...");
    delay(1000);
    if( !LittleFS.begin() ) {
      Log("mounting failed twice, formatting and then restarting");
      LittleFS.format();
      ESP.restart();
    } else {
      Log("Filesystem mounted at second try, proceeding as usual");
    }
  }

  // read configuration file
  Log("reading config from file or use default values");
  readConfig();

  // apply brightness and ratio from config
  g_brightness = configuration.brightness;
  g_ratio = configuration.ratio;

  // apply hostname
  wifi_station_set_hostname(configuration.hostname);
  
  Log("initializing LEDs");
  setup_LEDs();
  
  Log("initializing WiFi");
  setup_wifi();
  
  Log("initializing Webserver");
  setup_webserver();
  
  Log("initializing knob");
  setup_knob();

  state = CONSTANTCOLOR;
}

/******************************************************************************
Description.: execute the different subtasks, none may block or not return
Input Value.: -
Return Value: -
******************************************************************************/
void loop(void) {
  loop_wifi();
  loop_webserver();
  loop_LEDs();
  loop_knob();

  if(state == RESET_CONFIGURATION) {
    Log("deleting configuration file /config.json");
    
    LittleFS.remove(CONFIG_FILE);

    //keep running for ~5 more seconds, then restart
    for(int i=0; i<5000; i++) {
      loop_wifi();
      loop_webserver();
      loop_LEDs();

      // slow down this loop and yield at once
      delay(1);
    }
    ESP.restart();
  }
}
