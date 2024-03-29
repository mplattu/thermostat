#ifndef _TEMP_SENSOR_LISTENER_H_
#define _TEMP_SENSOR_LISTENER_H_

#include <bits/stdc++.h>
#include <WiFiUdp.h>

class TempSensorListener {
public:
    TempSensorListener(int udpPort);
    void begin();
    void readMessages();
    float getAverageTemperature();
private:
    bool storeSensorValue(String message);
    int udpPort;
    WiFiUDP wifiUdp;
    std::map<String, float> sensorValues;
};

#endif