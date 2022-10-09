#include <Arduino.h>
#include "influxDbTalker.h"

InfluxDbTalker::InfluxDbTalker() {
    this->influxDbClientInitialised = false;
}

InfluxDbTalker::InfluxDbTalker(
        const char *influxDbSensorName,
        const char *influxDbUrl,
        const char *influxDbToken,
        const char *influxDbOrg,
        const char *influxDbBucket
    ) {
    this->sensorName = String(influxDbSensorName);
    this->influxDbClient = new InfluxDBClient(influxDbUrl, influxDbOrg, influxDbBucket, influxDbToken);
    this->lastErrorMessage = String("");
    this->influxDbClientInitialised = true;
}

bool InfluxDbTalker::begin() {
    if (!this->influxDbClientInitialised) {
        this->lastErrorMessage = String("InfluxDbTalker has not been initialised");
        return false;
    }

    if (!this->influxDbClient->validateConnection()) {
        this->lastErrorMessage = String("InfluxDB server connection failed: ") +
            String(this->influxDbClient->getLastErrorMessage());
        return false;
    }

    // TBD: Set time from NTP

    return true;
}

bool InfluxDbTalker::report(char* fieldName, float value) {
    if (!this->influxDbClientInitialised) {
        this->lastErrorMessage = String("InfluxDbTalker has not been initialised");
        return false;
    }

    Point influxDbSensor = Point(this->sensorName.c_str());

    influxDbSensor.clearFields();
    influxDbSensor.addField(fieldName, value);
    this->influxDbClient->pointToLineProtocol(influxDbSensor);
    if (!this->influxDbClient->writePoint(influxDbSensor)) {
        this->lastErrorMessage = String(this->influxDbClient->getLastErrorMessage());
        return false;
    }

    this->lastErrorMessage = String("");
    return true;
}

bool InfluxDbTalker::report(char* fieldName, String value) {
    if (!this->influxDbClientInitialised) {
        this->lastErrorMessage = String("InfluxDbTalker has not been initialised");
        return false;
    }

    Point influxDbSensor = Point(this->sensorName.c_str());

    influxDbSensor.clearFields();
    influxDbSensor.addField(fieldName, value);
    this->influxDbClient->pointToLineProtocol(influxDbSensor);
    if (!this->influxDbClient->writePoint(influxDbSensor)) {
        this->lastErrorMessage = String(this->influxDbClient->getLastErrorMessage());
        return false;
    }

    this->lastErrorMessage = String("");
    return true;
}

String InfluxDbTalker::getLastErrorMessage() {
    return this->lastErrorMessage;
}
