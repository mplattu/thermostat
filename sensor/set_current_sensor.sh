#!/bin/sh

new_sensor_configuration=$1
new_sensor_configuration_file=include/sensor_settings_${new_sensor_configuration}.cpp

echo "New sensor configuration: ${new_sensor_configuration}"

if [ -f ${new_sensor_configuration_file} ]; then
    cp ${new_sensor_configuration_file} include/sensor_settings.cpp
    echo "New configuration file: ${new_sensor_configuration_file}"
else
    echo "Given configuration file does not exist: ${new_sensor_configuration_file}"
    exit 1
fi
