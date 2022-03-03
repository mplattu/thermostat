#!/usr/bin/env python3

import os
from Logger import Logger
from Settings import Settings
from TemperatureMessages import TemperatureMessages
from TemperatureFMI import TemperatureFMI
from ESPCommander import ESPCommander
import BlynkLib

logger = Logger()
settings = Settings()

BLYNK_AUTH = settings.get_environ('BLYNK_AUTH_TOKEN')
BLYNK_PIN_TEMP_INDOOR = 0
BLYNK_PIN_TEMP_OUTDOOR = 3
BLYNK_PIN_FORCE_ON = 1
BLYNK_PIN_FORCE_OFF = 2

bcast_ip = ""
if settings.get_environ('SENSOR_BCAST_IP') != "":
    bcast_ip = settings.get_environ('SENSOR_BCAST_IP', False)

delay_between = 30
if settings.get_environ('THERMOSTAT_DELAY_BETWEEN') != "":
    delay_between = int(settings.get_environ('THERMOSTAT_DELAY_BETWEEN'))

room_temp_meter = TemperatureMessages(3490, bcast_ip)

blynk = BlynkLib.Blynk(BLYNK_AUTH)

forced_relay_position = None
@blynk.on("V*")
def blynk_handle_vpins(pin, value):
    logger.debug("Blynk pins: V{} value: {}".format(pin, value))
    if pin == BLYNK_PIN_FORCE_ON and value > 0:
        logger.debug("Heating forced on")
        forced_relay_position = True
    elif pin == BLYNK_PIN_FORCE_OFF and value > 0:
        logger.debug("Heating forced off")
        forced_relay_position = False
    else:
        forced_relay_position = None

while True:
    # Gather temperature readings for 30 seconds
    room_temp = room_temp_meter.get_temperature(room_temp_meter.gather_messages(10))
    if room_temp != None:
        logger.debug('Room temperature: %.2f' % room_temp)
        #blynk.virtual_write(BLYNK_PIN_TEMP_INDOOR, room_temp)
        blynk.virtual_write(BLYNK_PIN_TEMP_INDOOR, 20)
    else:
        logger.print('Did not get room temperature')

    blynk.run()
    #blynk.sync_virtual(BLYNK_PIN_FORCE_ON, BLYNK_PIN_FORCE_OFF)

    outdoor_temp_meter = TemperatureFMI(settings.get_environ('THERMOSTAT_FMI_URL', True))
    outdoor_temp = outdoor_temp_meter.get_temperature()
    if outdoor_temp != None:
        logger.debug('Outdoor temperature: %.2f' % outdoor_temp)
        #blynk.virtual_write(BLYNK_PIN_TEMP_OUTDOOR, outdoor_temp)
        blynk.virtual_write(BLYNK_PIN_TEMP_OUTDOOR, 25)
    else:
        logger.print('Did not get outdoor temperature')

    temp_diff = float(settings.get_environ('THERMOSTAT_TEMP_DIFF', True))
    target_temp = outdoor_temp + temp_diff
    logger.debug('Target temperature: %.2f' % target_temp)

    max_temp = None
    if settings.get_environ('THERMOSTAT_TEMP_MAX', False):
        max_temp = float(settings.get_environ('THERMOSTAT_TEMP_MAX', False))

    esp_commander = ESPCommander(
        settings.get_environ('THERMOSTAT_DEVICE', True),
        settings.get_environ('THERMOSTAT_DEVICE_PASSWORD', True),
        settings.get_environ('THERMOSTAT_DEVICE_KEY', True)
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
    elif (max_temp is not None and room_temp > max_temp):
        # Max temp set and reached
        esp_commander.relay_off()
        relay_status = 'OFF (Over max)'
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

    time.sleep(delay_between)
