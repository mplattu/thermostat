# thermostat

Thermostat which drives the room temperature +2 degrees above the outside temp.

These are the building blocks for the project:
 * Raspberry Pi 3 running balenaCloud IoT environment for easy remote access & updates
 * Local network with 3G-WLAN hotspot
 * Room temperature with Wemos D1 & DS18B20 thermocouple sensor
 * Outdoor temperature from Finnish Meteorological Institute
 * WLAN Relay Deltaco Smartplug SH-P01 [reflashed](http://io.sivuduuni.biz/reflashing-deltaco-smartplug-sh-p01-to-work-with-home-assistant/) with ESPhome. The relay can be controlled with ESPhome [REST API](https://esphome.io/web-api/index.html#switch).

How this works:
 1. balenaCloud uploads and executes the code in the Raspberry Pi (Server)
 1. Server gets the outdoor temperature from FMI API
 1. Server gets the indoor temperature from a Wemos D1 sensor which broadcasts temperature to LAN every two seconds
 1. Server runs the SH-P01 relay(s) via WiFi

## Sensor

To build a sensor you need:
 * Wemos D1 microcontroller
 * DS18B20 temperature sensor
 * 4,7 kOhm resistor
 * PlatformIO build environment

Connect the Wemos D1 pins (Dn) to DS18B20 sensor in a following manner:

```
Wemos D1   DS18B20
 3,3        VCC (red)
 G          GND (black)
 D7         data (yellow)
```
* Connect Wemos D1 pins `3,3` and `D7` with 4,7 kOhm resistor

Building:
 1. Configure your sensor by editing `include/settings.cpp` and `sensor/include/settings.cpp` (get the templates from `settings.cpp.sample` files).
    The best practice is to create sensor-specific file `sensor/include/settings_mysensor.cpp` and activate this with `sensor/set_current_sensor.sh`.
    
    By using OTA DRIVE cloud service you can set only the `OTA_DRIVE_APIKEY` value in the
    `sensor/include/settings.cpp` and download rest values from the service. This way you can set all
    device-specific values as an OTA DRIVE configuration and use same firmware for all your sensors.

 1. Build, upload and execute the firmware:
    ```
    cd sensor
    ./set_current_sensor.sh mysensor
    make upload
    ```
 1. Check Wemos D1 led:
  * Blinking quickly (200 ms cycle): trying to connect to WiFi
  * Blinking slowly (2000 ms cycle): sending temperature data as UDP packets
 1. Dump the broadcast messages using `tcpdump`:
    ```
    $ sudo tcpdump -A port 3490
    tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
    listening on eno1, link-type EN10MB (Ethernet), capture size 262144 bytes
    10:09:59.678357 IP 192.168.49.101.3490 > 192.168.49.255.3490: UDP, length 14
    E..*.x........1e..1.........Sensor 1:24.00....
    10:10:01.719360 IP 192.168.49.101.3490 > 192.168.49.255.3490: UDP, length 14
    E..*.y........1e..1.........Sensor 1:24.75....
    ```

### Error Codes

In case of error the internal led is on for 5 seconds. After this it shows the error
code with 0,5 second blinks. See `grep showError sensor/src/sensor.cpp` for error
codes.

### Configuration and OTA Update Using OTA DRIVE

OTA DRIVE is an cloud service for updating and configuring MCUs. Temperature sensors
can be updated and configured through the service.

 * In `include/settings.cpp` set the `OTA_DRIVE_APIKEY` which you get from your
   OTA DRIVE product.
 * Set following JSON to the default configuration:
   ```
   {
    "sensorName": "<!--not-set-->",
    "influxDbToken": "<!---->",
    "deepSleepSeconds": "<!--300-->",
    "ledBlink": "<!--1-->"
   }
   ```
 * Add new devices one by one. This way you can add individual sensor names and
   InfluxDB tokens for each device from the device settings.
 * Precedence of the values:
   1. OTA DRIVE device-specific values
   1. OTA DRIVE default values
   1. firmware values from `sensor/include/settings.cpp`

To deploy a new firmware version
 * Update `SENSOR_FW_VERSION` string in the top of `sensor/src/sensor.cpp`
 * In `sensor/`, run `make build` to build the new firmware
 * Upload `.pio/build/wemosd1/firmware.bin` to OTA DRIVE
 * Assign the new version to your device group
 * Enjoy monitoring successfull upgrades

## Configuration

The parameters for the script are given as balenaCloud variables which are shown to the
application as environment vars.

Essential variables:

 * `THERMOSTAT_DELAY_BETWEEN` Delay between execution cycles in seconds. Type: integer
 * `THERMOSTAT_TEMP_DIFF` The desired temperature difference between outdoor and
   indoor temperatures. A positive value sets indoor temperature higher than the
   outdoor temperature. Type: integer
 * `SENSOR_BCAST_IP` Broadcast IP to listen for sensor messages. This should be a
    broadcast IP of your LAN (e.g. 192.168.0.255). If empty, binds to all addresses.
    Type: string

Defining inputs (ESP8266 sensors):

 * `SENSOR_INDOOR` Comma-separated list of indoor temperature sensors. These names refer to
   the names broadcasted by the temperature sensors (see above). Type: comma-separated list
   of strings
 * `SENSOR_OUTDOOR` Similair comma-separated list of outdoor temperature sensors. Tyoe:
   comma-separated list of strings

Defining outputs (ESPhome relays):

 * `RELAY_NAME` ESPhome mDNS device name (e.g. `relay_4.local`). Each device must
   have a key and API password defined. See below. Type: comma-separated
   list of strings
 * `RELAY_KEY` Device key (e.g. `relay_4_deltaco_sh-p01`) of the
   particular input to control. The keys are enumerated to the debug
   log `/var/log/thermostat.log` after the device is connected. Type: comma-separated
   list of strings
 * `RELAY_PASSWORD` API password for the device. Type: comma-separated
   list of strings

Optional variables:

 * `THERMOSTAT_FORCE_ON` If contains a value (e.g. `yes`) the relay is forced
   ON/closed to turn the heating on regardless of the temperatures. Type: string
 * `THERMOSTAT_FORCE_OFF` If contains a value (e.g. `yes`) the relay is forced
   OFF/open to turn the heating off regardless of the temperatures. Type: string
 * `THERMOSTAT_TEMP_MAX` Maximum room temperature. If the indoor temperature raises
   above this limit the relay is always turned off. Type: integer

Nord Pool variables (optional):

To give different values to thermostat while the current Nord Pool electricity price is
low (related to other hours of the day) you can use following variables:

 * `NORDPOOL_AREA` Set this to your Nord Pool area (e.g. `FI`, `EE`). Type: string
 * `NORDPOOL_TZ` Timezone descriptor which can be given directly to [pytz](https://pythonhosted.org/pytz/).
   Type: string
 * `NORDPOOL_HOURS` Defines the number of cheapest hours of the day. Value 5
   searches five cheapest hours in the current day. Type: integer
 * `THERMOSTAT_TEMP_DIFF_CHEAP` Overrides the default `THERMOSTAT_TEMP_DIFF` value
   during the Nord Pool cheap hours. Type: integer

InfluxDB variables (optional):

The InfluxDB variables allow the server to send relay status to InfluxDB. By configuring
this and corresponding sensor variables you can send monitoring data to the defined
InfluxDB.

 * `INFLUXDB_URL` URL of the InfluxDB server (e.g. `https://your.influxdb:8086`). Type: string
 * `INFLUXDB_ORG` InfluxDB organisation. Type: string
 * `INFLUXDB_BUCKET` InfluxDB bucket. Type: string
 * `INFLUXDB_TOKEN` InfluxDB access token. The server needs write access to the given bucket. Type: string
 * `INFLUXDB_SSL_CERT_BASE64` InfluxDB server SSL certificate chain. See below how to get this. Type: string

To get the base64-encoded cert do following:
 1. Open your InfluxDB server with Firefox
 1. Click the lock icon (left of URL) > Connection secure > More information > View certificate > Download the certificate chain in PEM format as `cert.pem`
 1. `cat cert.pem | base64 -w0 >cert.b64`
 1. Copy `cert.b64` to `INFLUXDB_SSL_CERT_BASE64`

Variables for testing and debugging:

To run Thermostat without real temperature sensors set `SENSOR_INDOOR` and `SENSOR_OUTDOOR` to
`_test_`. You can set the phony temperature values using environment variables `TEST_TEMP_INDOOR` and
`TEST_TEMP_OUTDOOR`.

Similair magic `INFLUXDB_URL` value `_test_` puts InfluxDB reporter to test mode and just
prints the values to STDERR.

If there are no relays just set empty values to `RELAY_NAME`, `RELAY_KEY` and `RELAY_PASSWORD`.

Defining devices (example):

 * One device:
   `RELAY_NAME=relay_1.local RELAY_KEY=relay1_deltaco_sh-p01 RELAY_PASSWORD=relay1apipassword`
 * Two (or more) devices:
   `RELAY_NAME=relay_1.local,relay_2.local RELAY_KEY=relay1_deltaco_sh-p01,relay2_deltaco_sh-p01 RELAY_PASSWORD=relay1apipassword,relay2apipassword`

Nord Pool configuration (example):

The aim of the Nord Pool features is to allow thermostat to heat the house more during the
cheapers hours of the day and less during the more expensive time slots. This mechanism retrieves
the Nord Pool price data for the given area and calculates the cheapest hours.

```
THERMOSTAT_TEMP_DIFF=1
NORDPOOL_AREA=FI
NORDPOOL_TZ=EET
NORDPOOL_HOURS=8
THERMOSTAT_TEMP_DIFF=5
```

The above configuration heats the house more during the cheapest eight hours (difference between
outdoor and indoor temperature should be more than five degrees) and less during the 16 more
expensive hours.

You should follow the temperatures to find best values for your site. InfluxDB is perfect tool for
monitoring purposes.

## Short Balena Local Mode Command Summmary

Start by enabling Local Mode from your device. This can be done in the BalenaCloud Management UI (see [Balena docs](https://www.balena.io/docs/learn/develop/local-mode/)).

 * `sudo balena scan` gives you the device address in your local network, e.g. `7064e4d.local`
 * `balena push ADDRESS` pushes your local codebase to device, e.g. `balena push 7064e4d.local`
 * `sudo balena ssh ADDRESS MY-SERVICE` opens ssh connection inside the container.
   The default service name is `main`, e.g. `balena ssh 7064e4d.local main`

## Publishing to production (Balena)

 * Make sure you have installed `balena` CLI tool
 * Get your fleet name from balenaCloud UI
 * `balena login`
 * `balena push <your fleet name>`
 * In balenaCloud UI go to the fleet > Summary and make Releases selection to point
   to your latest release.