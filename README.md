# Desk-Lamp-Alternative-Firmware
An alternative firmware for the Mijia Xiaomi Desk Lamp

Inspiration
-----------
After finding [fvollmers](https://github.com/fvollmer/xiomi-desk-lamp-open-firmware) project, it was possible to create this firmware. Thanks for the inspiration and the helpful details like GPIOs and pinouts.

Features
--------
 * This alternative firmware compiles with the Arduino IDE.
 * It provides a webserver and URLs for controlling the light.
 * The rotary knob reacts on single click and toggles between OFF an ON

Initial upload of firmware
--------------------------
The initial upload requires the following steps:
1. Open the desk lamp. Three screws are hidden underneath the rubber. Slightly peel away the rubber and unfasten the screws.
2. Then look for the PCB-A located below the rotary knob, it carries the ESP8266 module. Unfasten the screws of that PCB-A, pull off the knob gently and turn the PCB-A around. To protect the knob, I wrapped some electrical tape around it to prevent scratches and used pliers to pull it upwards, away from the PCB-A.
3. Solder cables to `RX`, `TX` and `GPIO0`. `GND` is also required for programming, but depending on your programming cable this signal is easily accessible even when the PCB is put back in place. Soldering to the ESP8266 module will result in fragile connections. Because of that, I decided to provide cable support afterwards by applying hot-melt-glue, otherwise the connections would not last long when handled.
4. Put back the PCB-A, fasten the screws.
5. Attach a 3.3V USB to serial adapter cable. Connect cables `GND` to `GND`, cables `Tx` to `Rx` and cables `Rx` to `Tx`. If you want to check correct wiring, power up the lamp and use the serial-monitor for any activity. Most likely it will just print gibberish due to mismatched baudrate, but this is enough to see `Rx` and `Tx` should be wired correctly; if there is no activity when powering the lamp try swapping the `Rx` and `Tx` lines.
6. Connect GPIO0 to GND. Keep it pulled low until firmware has uploaded from the Arduino IDE.
7. Compile the firmware and upload from within the Arduino IDE.
8. Unconnect GPIO0 from GND, power-cycle the lamp and it should dim-up. The serial monitor should inform you about the startup (115200, 8N1).

Flashing updates via WiFi
-------------------------
Once the firmware has uploaded initially, you can reupload through WiFi using the BASH script (assuming the desk-lamp got the IP adress 192.168.2.99):
    
    # ./flash.sh 192.168.2.99

The stdout-messages should look as follows:
    
    ./flash.sh 192.168.2.99
    enableUpdates: 1
    <META http-equiv="refresh" content="15;URL=/">Update Success! Rebooting...

Monitoring log-messages
-----------------------
To monitor the log messages, use the `log.sh` BASH script. It refreshes the log messages every second.
    
    ./log.sh 192.168.22.99

Upload files for the webserver
------------------------------
The internal webserver serves files from SPIFFS. To fill SPIFFS the most convenient way is creating a subfolder `data` in the sketch-folder and upload with the provided BASH-file `upload.sh`.

Uploading all files from `./data` to SPIFFS:

    ./upload_files.sh 192.168.2.99
    
Upload a single file file to SPIFFS:

    ./upload_files.sh 192.168.2.99 myfile 

Web-API
-------
To send commands to the desk lamp or read the state, the following HTTP-handlers are implemented:
 * Set color to 100% warmwhite: `http://192.168.2.99/color?warmwhite=255&coldwhite=0`
 * Get color as JSON: `http://192.168.2.99/color`
 * Get current state as JSON: `http://192.168.2.99/all`
 * Get log messages: `http://192.168.2.99/log`
 * Reset / Restart the lamp: `http://192.168.2.99/reset`
 * Retrieve the config as JSON (does not expose the WiFi credentials): `http://192.168.2.99/config.json`
 * There are more URLs to flash the firmware and upload files for the webserver (namely: `unlock`, `edit`, `format`, `update`). To prevent unauthorized flashing of (malicious) firmware or webserver files at least a hardcoded password is required. If you worry about this password change it in the sourcedode and notice that HTTP does not encrypt it.
