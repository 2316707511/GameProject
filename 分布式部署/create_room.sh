#!/bin/bash

docker run --rm -d -P -e MY_ROOM_NO=$1 game:v1.0.0 2>&1 >/dev/null

docker ps -n 1 -f ancestor=game:v1.0.0 | grep game_start | awk -F '->' '{print $1}' | awk -F ':' '{print $NF}'
