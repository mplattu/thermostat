#!/usr/bin/env python3

import os
from Logger import Logger
from Settings import Settings
from TemperatureMeter import TemperatureMeter
from TemperatureFMI import TemperatureFMI
from ESPCommander import ESPCommander

logger = Logger()
settings = Settings()

room_temp_meter = TemperatureMeter(settings.get_environ('THERMOSTAT_OWFS_ROOT', True), settings.get_environ('THERMOSTAT_OWFS_OVERRIDE'))
room_temp = room_temp_meter.get_temperature()
if room_temp != None:
    logger.print('Room temperature: %.2f' % room_temp)
else:
    logger.print('Did not get room temperature')

outdoor_temp_meter = TemperatureFMI(settings.get_environ('THERMOSTAT_FMI_URL', True))
outdoor_temp = outdoor_temp_meter.get_temperature()
if outdoor_temp != None:
    logger.print('Outdoor temperature: %.2f' % outdoor_temp)
else:
    logger.print('Did not get outdoor temperature')

temp_diff = float(settings.get_environ('THERMOSTAT_TEMP_DIFF', True))
target_temp = outdoor_temp + temp_diff
logger.print('Target temperature: %.2f' % target_temp)

esp_commander = ESPCommander(
    settings.get_environ('THERMOSTAT_DEVICE', True),
    settings.get_environ('THERMOSTAT_DEVICE_PASSWORD', True),
    settings.get_environ('THERMOSTAT_DEVICE_KEY', True)
)

relay_status = 'n/a'
if (target_temp < room_temp):
    # Too warm inside
    esp_commander.relay_off()
    relay_status = 'OFF'
else:
    # Too cold inside
    esp_commander.relay_on()
    relay_status = 'ON'

logger.print(relay_status)

print('CSV\t%s\t%.2f\t%.2f\t%.2f\t%s' % (logger.get_timestamp(), room_temp, outdoor_temp, target_temp, relay_status))
