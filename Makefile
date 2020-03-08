test-run:
	THERMOSTAT_FMI_URL="http://opendata.fmi.fi/wfs?service=WFS&version=2.0.0&request=getFeature&storedquery_id=fmi::forecast::hirlam::surface::point::multipointcoverage&place=raseborg&parameters=Temperature" \
	THERMOSTAT_TEMP_DIFF=5 \
	THERMOSTAT_DEVICE=device \
	THERMOSTAT_DEVICE_PASSWORD=device.password \
	THERMOSTAT_DEVICE_KEY=device.key \
	python3 thermostat/thermostat.py --owfs test/owfs --test_mode
