# thermostat

Thermostat which drives the room temperature +2 degrees above the outside temp.

These are the building blocks for the project:
 * Raspberry Pi 3 running balenaCloud IoT environment for easy remote access & updates
 * Local network with 3G-WLAN hotspot
 * Room temperature with Wemos D1 & MAX6675 thermocouple sensor
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
 * MAX6675 temperature sensor and thermocouple
 * PlatformIO build environment

Connect the Wemos D1 pins (Dn) to MAX6675 in a following manner:
 * D5 – SCK
 * D6 – CS
 * D7 – S0
 * G – GND
 * VCC – 3V3

You don't need any additional components or wirings.

Building:
 1. Configure your sensor by editing `sensor/include/settings.cpp` (get a template from `sensor/include/settings.cpp.sample`)
 1. Build, upload and execute the firmware:
    ```
    cd sensor
    make run
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

## Configuration

The parameters for the script are given as balenaCloud variables which are shown to the
application as environment vars. Here is a list of variables:
 * `BLYNK_AUTH_TOKEN` Blynk authentication token. Type: string
 * `THERMOSTAT_DELAY_BETWEEN` Delay between execution cycles in seconds. Type: integer
 * `SENSOR_BCAST_IP` Broadcast IP to listen for sensor messages. This should be a
    broadcast IP of your LAN (e.g. 192.168.0.255). If empty, binds to all addresses.
    Type: string
 * `THERMOSTAT_DEVICE` ESPhome device name (e.g. `relay_4.local`). Each device must
   have a key and API password defined. See below. Type: comma-separated
   list of strings
 * `THERMOSTAT_DEVICE_KEY` Device key (e.g. `relay_4_deltaco_sh-p01`) of the
   particular input to control. The keys are enumerated to the debug
   log `/var/log/thermostat.log` after the device is connected. Type: comma-separated
   list of strings
 * `THERMOSTAT_DEVICE_PASSWORD` API password for the device. Type: comma-separated
   list of strings
 * `THERMOSTAT_FMI_URL` URL to the [FMI WFS API](https://en.ilmatieteenlaitos.fi/open-data-manual-fmi-wfs-services)
   (e.g. [this](http://opendata.fmi.fi/wfs?service=WFS&version=2.0.0&request=getFeature&storedquery_id=fmi::forecast::hirlam::surface::point::multipointcoverage&place=raseborg&parameters=Temperature) - check the nearest municipality and edit parameter `place=xxx`). Type: string
 * `THERMOSTAT_TEMP_DIFF` The desired temperature difference between outdoor and
   indoor temperatures. A positive value sets indoor temperature higher than the
   outdoor temperature. Type: integer
 * `THERMOSTAT_TEMP_MAX` Maximum room temperature. If the indoor temperature raises
   above this limit the relay is always turned off. Type: integer

Defining devices

 * One device:
   `THERMOSTAT_DEVICE=relay_1.local THERMOSTAT_DEVICE_KEY=relay1_deltaco_sh-p01 THERMOSTAT_DEVICE_PASSWORD=relay1apipassword`
 * Two (or more) devices:
   `THERMOSTAT_DEVICE=relay_1.local,relay_2.local THERMOSTAT_DEVICE_KEY=relay1_deltaco_sh-p01,relay2_deltaco_sh-p01 THERMOSTAT_DEVICE_PASSWORD=relay1apipassword,relay2apipassword`

## Note About Licenses

The [blynk-library-python](https://github.com/vshymanskyy/blynk-library-python)  files `thermostat/Blynk*.py`) license is at `thermostat/Blynk.LICENSE`.

## Short Balena Local Mode Command Summmary

Start by enabling Local Mode from your device. This can be done in the BalenaCloud Management UI (see [Balena docs](https://www.balena.io/docs/learn/develop/local-mode/)).

 * `sudo balena scan` gives you the device address in your local network, e.g. `7064e4d.local`
 * `balena push ADDRESS` pushes your local codebase to device, e.g. `balena push 7064e4d.local`
 * `sudo balena ssh ADDRESS MY-SERVICE` opens ssh connection inside the container.
   The default service name is `main`, e.g. `balena ssh 7064e4d.local main`
