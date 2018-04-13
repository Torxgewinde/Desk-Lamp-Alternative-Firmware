#!/bin/bash

IP="xiaomi_desk_lamp.fritz.box"

if [ $# -eq 1 ]; then
	IP="$1"
fi

curl http://$IP/unlock?password=securitybyobscurity
echo