#!/usr/bin/env python3

import os
from Logger import Logger
from Settings import Settings
from TemperatureMessages import TemperatureMessages
from ESPCommander import ESPCommander

logger = Logger()
settings = Settings()

# Check for forced relay position
forced_relay_position = None
if settings.environ_is_true('THERMOSTAT_FORCE_ON'):
    forced_relay_position = True
    logger.debug('Thermostat forced ON by environment variable')

if settings.environ_is_true('THERMOSTAT_FORCE_OFF'):
    forced_relay_position = False
    logger.debug('Thermostat forced OFF by environment variable')

bcast_ip = ""
if settings.get_environ('SENSOR_BCAST_IP') != "":
    bcast_ip = settings.get_environ('SENSOR_BCAST_IP', False)

sensor_names_indoor = settings.get_environ_as_list('SENSOR_INDOOR', True)
sensor_names_outdoor = settings.get_environ_as_list('SENSOR_OUTDOOR', True)

temperature_message_listener = TemperatureMessages(3490, bcast_ip)

# Gather indoor temperature readings for 30 seconds
temp_indoor = temperature_message_listener.get_temperature(
        temperature_message_listener.gather_messages(30, sensor_names_indoor)
    )
if temp_indoor != None:
    logger.debug('Indoor temperature: %.2f' % temp_indoor)
else:
    logger.print('Did not get indoor temperature')

# Gather outdoor temperature readings for 30 seconds
temp_outdoor = temperature_message_listener.get_temperature(
        temperature_message_listener.gather_messages(30, sensor_names_outdoor)
    )
if temp_outdoor != None:
    logger.debug('Outdoor temperature: %.2f' % temp_outdoor)
else:
    logger.print('Did not get outdoor temperature')

temp_diff = float(settings.get_environ('THERMOSTAT_TEMP_DIFF', True))
target_temp = temp_outdoor + temp_diff
logger.debug('Target temperature: %.2f' % target_temp)

max_temp = None
if settings.get_environ('THERMOSTAT_TEMP_MAX', False):
    max_temp = float(settings.get_environ('THERMOSTAT_TEMP_MAX', False))

esp_commander = ESPCommander(
    settings.get_environ('RELAY_NAME', True),
    settings.get_environ('RELAY_PASSWORD', True),
    settings.get_environ('RELAY_KEY', True)
)

relay_status = 'n/a'
if (forced_relay_position == True):
    # Forced on
    esp_commander.relay_on()
    relay_status = 'ON (Forced)'
elif (forced_relay_position == False):
    # Forced off
    esp_commander.relay_off()
    relay_status = 'OFF (Forced)'
elif (max_temp is not None and temp_indoor > max_temp):
    # Max temp set and reached
    esp_commander.relay_off()
    relay_status = 'OFF (Over max)'
elif (target_temp < temp_indoor):
    # Too warm inside
    esp_commander.relay_off()
    relay_status = 'OFF'
else:
    # Too cold inside
    esp_commander.relay_on()
    relay_status = 'ON'

logger.debug(relay_status)

logger.print('CSV\t%s\t%.2f\t%.2f\t%.2f\t%s' % (logger.get_timestamp(), temp_indoor, temp_outdoor, target_temp, relay_status))
