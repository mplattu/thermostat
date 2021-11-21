#!/bin/sh

# Start Avahi-daemon to resolve .local mDNS domain
service dbus start
service avahi-daemon start

LOG=/var/log/thermostat.log

# Start Thermostat script
cd /srv/thermostat/thermostat

# Loop forever
while [ 1 != 2 ]
do
  echo "--- now executing start-thermostat.sh" >>$LOG
  ./thermostat.py 2>>$LOG
  echo "--- executed start-thermostat.sh" >>$LOG
  sleep $THERMOSTAT_DELAY_BETWEEN
done
