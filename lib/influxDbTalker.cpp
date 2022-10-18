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
        const char *influxDbBucket,
        const char *influxDbServerCertSha1Fingerprint
    ) {
    this->sensorName = String(influxDbSensorName);
    this->influxDbClient = new InfluxDBClient(influxDbUrl, influxDbOrg, influxDbBucket, influxDbToken, influxDbServerCertSha1Fingerprint);
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

String InfluxDbTalker::getServerUrl() {
    return this->influxDbClient->getServerUrl();
}

bool InfluxDbTalker::report(char* fieldName, float value) {
    if (!this->influxDbClientInitialised) {
        this->lastErrorMessage = String("InfluxDbTalker has not been initialised");
        return false;
    }

    /*
    if (!this->influxDbClient->isConnected()) {
        this->lastErrorMessage = String("Not connected");
        return false;
    }
    */

    Point influxDbSensor = Point(this->sensorName.c_str());

    influxDbSensor.clearFields();
    influxDbSensor.addField(fieldName, value);
    Serial.print("Writing to InfluxDB (float): ");
    Serial.println(this->influxDbClient->pointToLineProtocol(influxDbSensor));
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

    /*
    if (!this->influxDbClient->isConnected()) {
        this->lastErrorMessage = String("Not connected");
        return false;
    }
    */
    
    Point influxDbSensor = Point(this->sensorName.c_str());

    influxDbSensor.clearFields();
    influxDbSensor.addField(fieldName, value);
    Serial.print("Writing to InfluxDB (String): ");
    Serial.println(this->influxDbClient->pointToLineProtocol(influxDbSensor));
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
