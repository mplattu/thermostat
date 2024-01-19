#include "otaConfiguration.h"

#include <otadrive_esp.h>
#include <ArduinoJson.h>

bool OtaConfiguration::updateConfiguration() {
    String payload = OTADRIVE.getConfigs();
    DynamicJsonDocument doc(512);
    deserializeJson(doc, payload);

    if (doc.containsKey("sensorName")) {
        sensorName = String(doc["sensorName"].as<const char *>());
    } else {
#ifdef SENSOR_NAME
        sensorName = String(SENSOR_NAME);
#else
        sensorName = "n/a";
#endif
    }

    if (doc.containsKey("influxDbToken")) {
        influxDbToken = String(doc["influxDbToken"].as<const char *>());
    } else {
#ifdef INFLUXDB_TOKEN
        influxDbToken = String(INFLUXDB_TOKEN);
#else
        influxDbToken = "n/a";
#endif
    }

    if (doc.containsKey("deepSleepSeconds")) {
        deepSleepSeconds = doc["deepSleepSeconds"].as<int>();
    } else {
#ifdef DEEP_SLEEP_SECONDS
        deepSleepSeconds = DEEP_SLEEP_SECONDS;
#else
        deepSleepSeconds = 0;
#endif
    }

    if (doc.containsKey("ledBlink")) {
        ledBlink = doc["ledBlink"].as<int>();
    } else {
        ledBlink = 1;
    }

    return true;
}
