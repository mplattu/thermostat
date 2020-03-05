#!/bin/sh

# Create 1-wire filesystem
mkdir /tmp/owfs
owfs -u -m /tmp/owfs

# Start Thermostat script
cd /srv/thermostat/thermostat

# Loop forever
while [ 1 != 2 ]
do
  ./thermostat.py --owfs /tmp/owfs >>/var/log/thermostat.log
  echo "start-thermostat.sh executed"
  sleep $THERMOSTAT_DELAY_BETWEEN
done
