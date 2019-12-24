#!/bin/bash

USERNAME=$1
PASSWORD=$2

PASSWORD=`echo -n $PASSWORD | openssl dgst -md5 -hex | awk '{print $NF}'`

echo "${USERNAME}:${PASSWORD}" >> userfile
