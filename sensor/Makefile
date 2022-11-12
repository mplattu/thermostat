build:
	pio run -e wemosd1

upload:
	pio run -e wemosd1 -t upload

upload_ota:
	@echo "Uploading to: $(OTA_ADDRESS)"
	if [ -z "$(OTA_ADDRESS)" ]; then echo "Environment variable OTA_ADDRESS is not defined"; exit 1; fi
	pio run -e wemosd1_ota -t upload

monitor:
	pio device monitor

clean:
	rm -fR .pio/
