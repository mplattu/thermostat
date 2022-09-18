#include "tempSensorListener.h"

TempSensorListener::TempSensorListener(int udpPort) {
    this->udpPort = udpPort;
}

void TempSensorListener::begin() {
    this->wifiUdp.begin(udpPort);
}

void TempSensorListener::readMessages() {
    char packet[255];
    String packetStr;

    // Reads incoming UDP messages
    int packetSize = this->wifiUdp.parsePacket();
    if (packetSize) {
        int len = this->wifiUdp.read(packet, 255);
        if (len > 0) {
            packet[len] = '\0';
        }

        packetStr = String(packet);

        bool storeSuccess = this->storeSensorValue(packetStr);
        if (not storeSuccess) {
            Serial.print("Failed to decode message: ");
            Serial.println(packetStr);
        }
    }
}

bool TempSensorListener::storeSensorValue(String message) {
    Serial.print("Storing: ");
    Serial.println(message);

    int separatorPosition = message.indexOf(":");
    if (separatorPosition == -1) {
        return false;
    }

    String sensorName = message.substring(0, separatorPosition);
    String sensorValueStr = message.substring(separatorPosition + 1);
    float sensorValue = sensorValueStr.toFloat();

    this->sensorValues[sensorName] = sensorValue;

    return true;
}

float TempSensorListener::getAverageTemperature() {
    std::map<String, float>::iterator i;

    int sensorCount = 0;
    float sumOfTemperatures = 0.0;

    Serial.print("getAverageTemperature: ");
    for (i = this->sensorValues.begin(); i != this->sensorValues.end(); i++) {
        Serial.print(i->first + "=" + i->second + " ");

        sensorCount = sensorCount + 1;
        sumOfTemperatures = sumOfTemperatures + i->second;
    }
    Serial.println();

    return sumOfTemperatures / sensorCount;
}