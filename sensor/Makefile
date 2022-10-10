set_current_to_sensor_1:
	ln -sf sensor_settings_sensor1.cpp include/sensor_settings.cpp

set_current_to_sensor_2:
	ln -sf sensor_settings_sensor2.cpp include/sensor_settings.cpp

set_current_to_sensor_test:
	ln -sf sensor_settings_test.cpp include/sensor_settings.cpp

build_current:
	pio run -e wemosd1

upload_current:
	pio run -e wemosd1 -t upload

build_sensor_1: set_current_to_sensor_1 build_current

build_sensor_2: set_current_to_sensor_2 build_current

build_sensor_test: set_current_to_sensor_test build_current

upload_sensor_1: set_current_to_sensor_1 upload_current

upload_sensor_2: set_current_to_sensor_2 upload_current

upload_sensor_test: set_current_to_sensor_test upload_current

monitor:
	pio device monitor

clean:
	rm -fR .pio/
