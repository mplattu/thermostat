# thermostat

Thermostat which drives the room temperature +2 degrees above the outside temp.

These are the building blocks for the project:
 * Raspberry Pi 3 running balenaCloud IoT environment for easy remote access & updates
 * Local network with 3G-WLAN hotspot
 * Room temperature with USB/1-Wire DS1820
 * Outdoor temperature from Finnish Meteorological Institute
 * WLAN Relay Deltaco Smartplug SH-P01 [reflashed](http://io.sivuduuni.biz/reflashing-deltaco-smartplug-sh-p01-to-work-with-home-assistant/) with ESPhome. The relay can be controlled with ESPhome [REST API](https://esphome.io/web-api/index.html#switch).

## Configuration

The parameters for the script are given as balenaCloud variables which are shown to the
application as environment vars. Here is a list of variables:
 * `THERMOSTAT_DELAY_BETWEEN` Delay between execution cycles in seconds. Type: integer
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
 * `THERMOSTAT_FORCE_ON` If contains a value (e.g. `yes`) the relay is forced
   ON/closed to turn the heating on regardless of the temperatures. Type: string
 * `THERMOSTAT_FORCE_OFF` If contains a value (e.g. `yes`) the relay is forced
   OFF/open to turn the heating off regardless of the temperatures. Type: string
 * `THERMOSTAT_OWFS_OVERRIDE` In normal setting all temperature meters in the 1-Wire
   network are read and their average temperature is used. Devices listed (their 1-Wire
   IDs, e.g. `10.67C6697351FF,10.67C669735F2C`) here ignore them. Type: comma-separated
   list of strings
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

## Short Balena Local Mode Command Summmary

Start by enabling Local Mode from your device. This can be done in the BalenaCloud Management UI (see [Balena docs](https://www.balena.io/docs/learn/develop/local-mode/)).

 * `sudo balena scan` gives you the device address in your local network, e.g. `7064e4d.local`
 * `balena push ADDRESS` pushes your local codebase to device, e.g. `balena push 7064e4d.local`
 * `sudo balena ssh ADDRESS MY-SERVICE` opens ssh connection inside the container.
   The default service name is `main`, e.g. `balena ssh 7064e4d.local main`
