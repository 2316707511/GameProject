#!/bin/bash

if [ "$2" == "" ]
then
	cat userfile | grep -w $1 2>&1 > /dev/null

else
	cat userfile | grep -w "$1:$2" 2>&1 >/dev/null
fi
