#!/bin/bash
./game daemon
while [ a == a ]
do
	sleep 10
	ps aux | grep -w game | grep -v grep
	if [ 0 -ne $? ]
	then
		redis-cli -h "122.51.68.107" -p 6379 del $MY_ROOM_NO
		exit 0
	fi
done
