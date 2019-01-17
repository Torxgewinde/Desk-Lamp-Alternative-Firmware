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
********************************************************************************

IDE and toolchain:
- Arduino 1.8.5
- ESP8266 Toolchain 2.4.2

Additional libraries:
- Arduino JSON, Version 5.13.4
- WiFiManager, Version 0.14.0
- Encoder, Version 1.4.1
- OneButton, Version 1.3.0

Hardware: 
- Xiaomi Desk Lamp

Arduino Settings:
- Generic ESP8266 Module
- CPU Frequency 80 Mhz
- Flash Size 1 M (64k SPIFFS)
- Flash Mode is QIO
- Crystal Frequency is 26 MHz
- Reset Method "ck"

To flash:
1. Attach a 3.3V USB to Serial adaptor to Rx, Tx, Gnd
2. Power off
3. Pull GPIO0 to GND
4. Power Up
5. upload firmware with Arduino IDE

*******************************************************************************/
