test-run:
	THERMOSTAT_FMI_URL="http://opendata.fmi.fi/wfs?service=WFS&version=2.0.0&request=getFeature&storedquery_id=fmi::forecast::hirlam::surface::point::multipointcoverage&place=raseborg&parameters=Temperature" \
	THERMOSTAT_TEMP_DIFF=3 \
	THERMOSTAT_DEVICE=relay_3_deltaco_sh-p01,relay_4_deltaco_sh-p01 \
	THERMOSTAT_DEVICE_PASSWORD=Xtt3uWWx26JRWQ9v,7MvfaPBg4N76b5FG \
	THERMOSTAT_DEVICE_KEY=relay_3_deltaco_sh-p01,relay_4_deltaco_sh-p01 \
	BLYNK_AUTH_TOKEN=ATJ6etm7YVdK6cqA1d9P5g0NTMNVg1qO \
	THERMOSTAT_DELAY_BETWEEN=5 \
	python3 thermostat/thermostat.py
