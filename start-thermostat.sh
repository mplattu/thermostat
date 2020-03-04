#!/bin/sh

# Create 1-wire filesystem
mkdir /tmp/owfs
owfs -u -m /tmp/owfs

# Start Thermostat script
cd /srv/thermostat/thermostat

# Loop forever
while [ 1 != 2 ]
do
  ./thermostat.py >>/var/log/thermostat.log
  echo "start-thermostat.sh executed"
  sleep 1800
done
