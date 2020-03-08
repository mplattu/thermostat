#!/usr/bin/env python3

import os
from Logger import Logger
from Settings import Settings
from TemperatureMeter import TemperatureMeter
from TemperatureFMI import TemperatureFMI
from ESPCommander import ESPCommander

logger = Logger()
settings = Settings()

# Check for forced relay position
forced_relay_position = None
if settings.get_environ('THERMOSTAT_FORCE_ON') != "":
    forced_relay_position = True
    logger.debug('Thermostat forced ON by environment variable')

if settings.get_environ('THERMOSTAT_FORCE_OFF') != "":
    forced_relay_position = False
    logger.debug('Thermostat forced OFF by environment variable')

room_temp_meter = TemperatureMeter(settings.get_cmdline('owfs_root', True), settings.get_environ('THERMOSTAT_OWFS_OVERRIDE'))
room_temp = room_temp_meter.get_temperature()
if room_temp != None:
    logger.debug('Room temperature: %.2f' % room_temp)
else:
    logger.print('Did not get room temperature')

outdoor_temp_meter = TemperatureFMI(settings.get_environ('THERMOSTAT_FMI_URL', True))
outdoor_temp = outdoor_temp_meter.get_temperature()
if outdoor_temp != None:
    logger.debug('Outdoor temperature: %.2f' % outdoor_temp)
else:
    logger.print('Did not get outdoor temperature')

temp_diff = float(settings.get_environ('THERMOSTAT_TEMP_DIFF', True))
target_temp = outdoor_temp + temp_diff
logger.debug('Target temperature: %.2f' % target_temp)

esp_commander = ESPCommander(
    settings.get_environ('THERMOSTAT_DEVICE', True),
    settings.get_environ('THERMOSTAT_DEVICE_PASSWORD', True),
    settings.get_environ('THERMOSTAT_DEVICE_KEY', True)
)

esp_commander.set_test_mode(settings.get_cmdline('test_mode'))

relay_status = 'n/a'
if (forced_relay_position == True):
    # Forced on
    esp_commander.relay_on()
    relay_status = 'ON (Forced)'
elif (forced_relay_position == False):
    # Forced off
    esp_commander.relay_off()
    relay_status = 'OFF (Forced)'
elif (target_temp < room_temp):
    # Too warm inside
    esp_commander.relay_off()
    relay_status = 'OFF'
else:
    # Too cold inside
    esp_commander.relay_on()
    relay_status = 'ON'

logger.debug(relay_status)

logger.print('CSV\t%s\t%.2f\t%.2f\t%.2f\t%s' % (logger.get_timestamp(), room_temp, outdoor_temp, target_temp, relay_status))
