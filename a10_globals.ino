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

#include <ArduinoJson.h>
#include <FS.h>
#include <deque>

//determine array length
#define LENGTH_OF(x) (sizeof(x)/sizeof(x[0]))

/* current state of the device */
enum STATES {UNDEF=0, CONSTANTCOLOR, BOOTUP, FLASH, LIGHTSOFF, RESET_CONFIGURATION};
struct {
  STATES state;
  String human_readable_string;
  String state_as_string;
} state_map[] = {
  { UNDEF, "Undefined State", "undef" },
  { CONSTANTCOLOR,"Static Color Mode", "constantcolor" },
  { BOOTUP, "Booting up...", "bootup" },
  { FLASH, "Flashing Firmware...", "flash" },
  { LIGHTSOFF, "Lights switched off", "lightsoff" },
  { RESET_CONFIGURATION, "Resetting all configurations", "reset_configuration" }
};
STATES state = UNDEF;

/* Variables for the warm and cold white LEDs */
float g_brightness, g_ratio;

/* only allow to write to flash if commando was set */
bool g_enableUpdates = false;

/* Storage for most recent logging messages */
std::deque<String> log_messages;

#define LOG_LENGTH 100

/* logging messages, maintain length of entries */
void Log(String text) {
  log_messages.push_back(text);
  log_messages.pop_front();

  Serial.println(text);
}

// configuration, values are either from SPIFFS or default values
struct {
  char hostname[64];
  float ratio, brightness;
  bool send_WLAN_keep_alive_packet;
  bool disable_WiFi;
} configuration;

/******************************************************************************
Description.: read config file from SPIFFS
Input Value.: -
Return Value: -
******************************************************************************/
void readConfig() {
  File configFile = SPIFFS.open("/config.json", "r");

  StaticJsonBuffer<512> jsonBuffer;
  
  JsonObject &root = jsonBuffer.parseObject(configFile);
  
  if (!root.success())
    Log("Failed to read config file /config.json from SPIFFS, using default values");

  strlcpy(configuration.hostname, root["hostname"] | "XIAOMI-DESK-LAMP", sizeof(configuration.hostname));
  configuration.ratio = root["ratio"] | 1.0;
  configuration.brightness = root["brightness"] | 1.0;
  configuration.send_WLAN_keep_alive_packet = root["send_WLAN_keep_alive_packet"] | true;
  configuration.disable_WiFi = root["disable_WiFi"] | false;

  configFile.close();
}

/******************************************************************************
Description.: save config file to SPIFFS
Input Value.: -
Return Value: -
******************************************************************************/
void writeConfig() {

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Log("Could not open config file in SPIFFS");
    return;
  }

  StaticJsonBuffer<512> jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  root["hostname"] = configuration.hostname;
  root["ratio"] = configuration.ratio;
  root["brightness"] = configuration.brightness;
  root["send_WLAN_keep_alive_packet"] = configuration.send_WLAN_keep_alive_packet;
  root["disable_WiFi"] = configuration.disable_WiFi;

  // Serialize JSON to file
  if (root.printTo(configFile) == 0) {
    Log("Could not store config in SPIFFS");
  }

  configFile.close();
}

