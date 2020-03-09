#!/bin/sh

LOG=/var/log/thermostat.log

# Create 1-wire filesystem
mkdir /tmp/owfs
owfs -u -m /tmp/owfs

# Start Thermostat script
cd /srv/thermostat/thermostat

# Loop forever
while [ 1 != 2 ]
do
  echo "--- now executing start-thermostat.sh" >>$LOG
  ./thermostat.py --owfs /tmp/owfs 2>>$LOG
  echo "--- executed start-thermostat.sh" >>$LOG
  sleep $THERMOSTAT_DELAY_BETWEEN
done
