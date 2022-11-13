test-run-nordpool-cheap:
	RELAY_NAME= \
	RELAY_KEY= \
	RELAY_PASSWORD= \
	THERMOSTAT_TEMP_DIFF=2 \
	SENSOR_INDOOR=_test_ \
	SENSOR_OUTDOOR=_test_ \
	TEST_TEMP_INDOOR=14 \
	TEST_TEMP_OUTDOOR=4 \
	NORDPOOL_AREA=FI \
	NORDPOOL_TZ=EET \
	NORDPOOL_HOURS=0 \
	THERMOSTAT_TEMP_DIFF_CHEAP=5 \
	python3 thermostat/thermostat.py

test-run-nordpool-expensive:
	RELAY_NAME= \
	RELAY_KEY= \
	RELAY_PASSWORD= \
	THERMOSTAT_TEMP_DIFF=2 \
	SENSOR_INDOOR=_test_ \
	SENSOR_OUTDOOR=_test_ \
	TEST_TEMP_INDOOR=14 \
	TEST_TEMP_OUTDOOR=4 \
	NORDPOOL_AREA=FI \
	NORDPOOL_TZ=EET \
	NORDPOOL_HOURS=0 \
	THERMOSTAT_TEMP_DIFF_CHEAP=5 \
	python3 thermostat/thermostat.py

test-run-nordpool-influxdb:
	RELAY_NAME= \
	RELAY_KEY= \
	RELAY_PASSWORD= \
	THERMOSTAT_TEMP_DIFF=2 \
	SENSOR_INDOOR=_test_ \
	SENSOR_OUTDOOR=_test_ \
	TEST_TEMP_INDOOR=14 \
	TEST_TEMP_OUTDOOR=4 \
	NORDPOOL_AREA=FI \
	NORDPOOL_TZ=EET \
	NORDPOOL_HOURS=0 \
	THERMOSTAT_TEMP_DIFF_CHEAP=5 \
	INFLUXDB_URL=_test_ \
	python3 thermostat/thermostat.py
