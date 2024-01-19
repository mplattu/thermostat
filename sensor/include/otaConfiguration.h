#ifndef OTA_CONFIGURATION_H
#define OTA_CONFIGURATION_H

class OtaConfiguration {
    public:
        bool updateConfiguration();

        String sensorName;
        String influxDbToken;
        int deepSleepSeconds;
        int ledBlink;
};

#endif
