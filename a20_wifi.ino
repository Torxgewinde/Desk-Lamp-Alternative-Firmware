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

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
uint8_t buffer[10] = {1};

/******************************************************************************
Description.: prepare the WiFi settings, uses WiFi Manager Library
Input Value.: -
Return Value: -
******************************************************************************/
void setup_wifi() {
  WiFiManager wifiManager;

  //reset settings - for testing
  //wifiManager.resetSettings();
  
  wifiManager.setConfigPortalTimeout(600);
  wifiManager.autoConnect("Xiaomi Desk Lamp");

  Log("WiFi connected, RSSI: "+ String(WiFi.RSSI()));
}

/******************************************************************************
Description.: no permanent task necessary
Input Value.: -
Return Value: -
******************************************************************************/
void loop_wifi() {
  //because of having issues with an unresposive connection
  //just meaningless data traffic will hopefully fix it
  //a single UDP packet is send to the gateway IP
  static unsigned long then = 0;
  static bool reset_config = false;

  // for using millis be aware of overflow every ~50 days
  // but using substraction is "overflow-safe"
  if( g_send_WLAN_keep_alive_packet && (millis()-then >= 30000) ) {
    then = millis();

    //UDP port 9 is supposed to discard packets or it will simply not return
    //anything because no service is running at the gateway-IP,port9
    Udp.beginPacket(WiFi.gatewayIP(), 9);
    Udp.write(buffer, LENGTH_OF(buffer));
    Udp.endPacket();

    Log("WiFi kept busy, RSSI: "+ String(WiFi.RSSI()) +", Connected: "+ String(WiFi.isConnected()));
  }

  // delete all WiFi Manager settings, but only erase it once
  if(state == RESET_CONFIGURATION && !reset_config) {
    Log("Resetting WiFi Manager");
    
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    
    reset_config = true;
  }
}
