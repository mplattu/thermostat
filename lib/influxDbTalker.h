#ifndef _TEMP_INFLUXDB_TALKER_H_
#define _TEMP_INFLUXDB_TALKER_H_

#include <ESP8266HTTPClient.h>
#include <InfluxDbClient.h>
InfluxDBClient influxDbClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

#ifdef SENSOR_NAME
Point influxDbSensor(SENSOR_NAME);
#endif

#ifdef SERVER_NAME
Point influxDbSensor(SERVER_NAME);
#endif

class InfluxDbTalker {
public:
    InfluxDbTalker();
    InfluxDbTalker(const char *influxDbSensorName,
            const char *influxDbUrl,
            const char *influxDbToken,
            const char *influxDbOrg,
            const char *influxDbBucket
        );
    bool begin();
    bool report(char* fieldName, float value);
    bool report(char* fieldName, String value);
    String getLastErrorMessage();
private:
    InfluxDBClient *influxDbClient;
    String lastErrorMessage;
    String sensorName;
    bool influxDbClientInitialised;
};

#include "influxDbTalker.cpp"

#endif
