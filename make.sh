#!/bin/bash

#Arduino IDE
ARDUINO_VERSION_TO_USE="1.8.13"

# Use this toolchain version
ESP8266_TOOLCHAIN_VERSION_TO_USE=2.7.4

# Install the following, additional libraries
ARDUINO_LIBRARIES_TO_USE="ArduinoJson:6.17.1,WiFiManager:0.15.0,Encoder:1.4.1,OneButton:1.5.0"
    
# Values for boards are derived from boards.txt file (https://github.com/esp8266/Arduino/blob/2.7.4/boards.txt)
BD="esp8266:esp8266:esp8285:CrystalFreq=26,xtal=80,ResetMethod=ck,eesz=2M64,dbg=Disabled,ip=src,led=2,ResetMethod=ck,wipe=all"

#IDE folder
IDE_FOLDER="/tmp/arduino"
BUILD_FOLDER="$(pwd)/build"

export PATH=$PATH:$IDE_FOLDER

#download, unpack and install Arduino-IDE + toolchain + libs
if [ ! -d $IDE_FOLDER ]; then
	if [ ! -r arduino-$ARDUINO_VERSION_TO_USE-linux64.tar.xz ]; then
		wget https://downloads.arduino.cc/arduino-$ARDUINO_VERSION_TO_USE-linux64.tar.xz || exit 1
	fi

	tar xf arduino-$ARDUINO_VERSION_TO_USE-linux64.tar.xz || exit 1
	mv arduino-$ARDUINO_VERSION_TO_USE $IDE_FOLDER || exit 1

	arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --pref "sketchbook.path=." --save-prefs
	arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --pref "build.path=$BUILD_FOLDER" --save-prefs
	arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --pref "compiler.warning_level=all" --save-prefs
	arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --pref "boardsmanager.additional.urls=https://arduino.esp8266.com/stable/package_esp8266com_index.json" --save-prefs
	arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --board $BD --save-prefs

	#download and install toolchain
	arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --install-boards esp8266:esp8266:$ESP8266_TOOLCHAIN_VERSION_TO_USE

	#download and install libraries
	arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --install-library $ARDUINO_LIBRARIES_TO_USE
fi

#compile the INO file
#arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --pref build.extra_flags="-DBLA=\"TEST\"" --verify --board $BD Desk*.ino
arduino --preferences-file $IDE_FOLDER/portable/preferences.txt --board $BD --verify Desk*.ino

#rm -rf $IDE_FOLDER
#rm arduino-$ARDUINO_VERSION_TO_USE-linux64.tar.xz || exit 1

exit 0
