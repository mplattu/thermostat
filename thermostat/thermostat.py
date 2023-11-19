#!/usr/bin/env python3

import os
from Logger import Logger
from Settings import Settings
from TemperatureMessages import TemperatureMessages
from ESPCommander import ESPCommander
from InfluxDbWriter import InfluxDbWriter
from NordPool import NordPool

INFLUXDB_RELAY_VALUES = {
    "on": 20,
    "on-forced": 25,
    "off": 10,
    "off-forced": 0,
    "off-over-max": 5
}

logger = Logger()
settings = Settings()

# Define InfluxDB (if set)
influxdb = InfluxDbWriter(
    "heating",
    settings.get_environ('INFLUXDB_URL'),
    settings.get_environ('INFLUXDB_ORG'),
    settings.get_environ('INFLUXDB_BUCKET'),
    settings.get_environ('INFLUXDB_TOKEN')
)
influxdb_legend = "relay"

# Define Nord Pool (if set)
nordpool = None
if settings.get_environ('NORDPOOL_AREA'):
    nordpool = NordPool(
        './nordpool.sqlite',
        settings.get_environ('NORDPOOL_AREA'),
        settings.get_environ('NORDPOOL_TZ', True)
    )

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
        temperature_message_listener.gather_messages(30, sensor_names_indoor, 'TEST_TEMP_INDOOR')
    )
if temp_indoor != None:
    logger.debug('Indoor temperature: %.2f' % temp_indoor)
else:
    logger.print('Did not get indoor temperature')

# Gather outdoor temperature readings for 30 seconds
temp_outdoor = temperature_message_listener.get_temperature(
        temperature_message_listener.gather_messages(30, sensor_names_outdoor, 'TEST_TEMP_OUTDOOR')
    )
if temp_outdoor != None:
    logger.debug('Outdoor temperature: %.2f' % temp_outdoor)
else:
    logger.print('Did not get outdoor temperature')

temp_diff = float(settings.get_environ('THERMOSTAT_TEMP_DIFF', True))
if nordpool is not None:
    nordpool.update_nordpool_data()
    if nordpool.is_cheap(int(settings.get_environ('NORDPOOL_HOURS'))):
        temp_diff = float(settings.get_environ('THERMOSTAT_TEMP_DIFF_CHEAP', True))
        logger.debug(f'Nord Pool: using cheap hours temp diff {temp_diff}')
    else:
        logger.debug('Nord Pool: We are not in the cheap hours')

    price_rank = nordpool.get_current_price_rank()
    if price_rank is not None:
        if not influxdb.write_value('nordpool_price_rank', price_rank):
            logger.print(f'Could not write NordPool price rank to InfluxDB server: {influxdb.last_error_message}')
    
    price_value = nordpool.get_current_price_value()
    if price_value is not None:
        if not influxdb.write_value('nordpool_price', price_value):
            logger.print(f'Could not write NordPool price to InfluxDB server: {influxdb.last_error_message}')


target_temp = temp_outdoor + temp_diff
logger.debug('Target temperature: %.2f' % target_temp)

max_temp = None
if settings.get_environ('THERMOSTAT_TEMP_MAX', False):
    max_temp = float(settings.get_environ('THERMOSTAT_TEMP_MAX', False))

esp_commander = ESPCommander(
    settings.get_environ_as_list('RELAY_NAME', True),
    settings.get_environ_as_list('RELAY_PASSWORD', True),
    settings.get_environ_as_list('RELAY_KEY', True)
)

relay_status = 'n/a'
influxdb_write_success = True

if (forced_relay_position == True):
    # Forced on
    esp_commander.relay_on()
    relay_status = 'ON (Forced)'
    influxdb_write_success = influxdb.write_value(influxdb_legend, INFLUXDB_RELAY_VALUES["on-forced"])
elif (forced_relay_position == False):
    # Forced off
    esp_commander.relay_off()
    relay_status = 'OFF (Forced)'
    influxdb_write_success = influxdb.write_value(influxdb_legend, INFLUXDB_RELAY_VALUES["off-forced"])
elif (max_temp is not None and temp_indoor > max_temp):
    # Max temp set and reached
    esp_commander.relay_off()
    relay_status = 'OFF (Over max)'
    influxdb_write_success = influxdb.write_value(influxdb_legend, INFLUXDB_RELAY_VALUES["off-over-max"])
elif (target_temp < temp_indoor):
    # Too warm inside
    esp_commander.relay_off()
    relay_status = 'OFF'
    influxdb_write_success = influxdb.write_value(influxdb_legend, INFLUXDB_RELAY_VALUES["off"])
else:
    # Too cold inside
    esp_commander.relay_on()
    relay_status = 'ON'
    influxdb_write_success = influxdb.write_value(influxdb_legend, INFLUXDB_RELAY_VALUES["on"])

if not influxdb_write_success:
    logger.print(f'Could not write relay status to InfluxDB: {influxdb.last_error_message}')

influxdb.close()

logger.debug(relay_status)

logger.print('CSV\t%s\t%.2f\t%.2f\t%.2f\t%s' % (logger.get_timestamp(), temp_indoor, temp_outdoor, target_temp, relay_status))
