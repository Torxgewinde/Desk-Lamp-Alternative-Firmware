#!/bin/bash

IP="xiaomi_desk_lamp.fritz.box"
FOLDER="data"

if [ $# -eq 2 ]; then
	echo "uploading: /$2"
	IP="$1"
	
	curl http://$IP/unlock?password=securitybyobscurity
	echo
	
	curl -F "file=@$FOLDER/$2;filename=/$2" $IP/edit
	exit
fi

if [ $# -eq 1 ]; then
	IP="$1"
fi

curl http://$IP/unlock?password=securitybyobscurity
echo

for i in $(find data -type f | sed -e "s/data\///g"); do
	echo "uploading: /$i"
	curl -F "file=@$FOLDER/$i;filename=/$i" $IP/edit
done
