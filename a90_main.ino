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
Description.: read config file from SPIFFS
Input Value.: -
Return Value: true if config read, false in case of error
******************************************************************************/
bool loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  
  if (!configFile || (configFile.size() > 1024)) {
    g_hostname = "XIAOMI_DESK_LAMP";
    state = CONSTANTCOLOR;
    g_WarmWhite = 255;
    g_ColdWhite = 0;
    g_send_WLAN_keep_alive_packet = true;
    
    return false;
  }

  int size = configFile.size();

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if (!json.success()) {
    return false;
  }

  g_hostname = strdup(json["hostname"]);
  g_send_WLAN_keep_alive_packet = json["send_WLAN_keep_alive_packet"];  
  g_WarmWhite = json["startupcolor_warmwhite"];
  g_ColdWhite = json["startupcolor_coldwhite"];

  return true;
}

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
  
  if( !SPIFFS.begin() ) {
    Log("SPIFFS not mounted correctly, retrying...");
    delay(1000);
    if( !SPIFFS.begin() ) {
      Log("mounting failed twice, formatting and then restarting");
      SPIFFS.format();
      ESP.restart();
    } else {
      Log("SPIFFS mounted at second try, proceeding as usual");
    }
  }

  // read configuration file
  bool r = loadConfig();
  Log("loadConfig() --> result: "+String(r));

  // overwrite the keep_alive_paket_config
  //g_send_WLAN_keep_alive_packet = false;

  // apply hostname
  wifi_station_set_hostname((char *)g_hostname.c_str());
  
  setup_LEDs();
  setup_wifi();
  setup_webserver();
  setup_knob();
}

/******************************************************************************
Description.: execute the different subtasks, none may block or not return
Input Value.: -
Return Value: -
******************************************************************************/
void loop(void){
  loop_wifi();
  loop_webserver();
  loop_LEDs();
  loop_knob();

  delay(1);
}

