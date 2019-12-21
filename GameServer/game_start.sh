#!/bin/bash
./game daemon
while [ a == a ]
do
	sleep 10
	ps aux | grep -w game | grep -v grep
	if [ 0 -ne $? ]
	then
		exit 0
	fi
done
