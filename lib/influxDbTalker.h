#ifndef _TEMP_INFLUXDB_TALKER_H_
#define _TEMP_INFLUXDB_TALKER_H_

#include <InfluxDbClient.h>
InfluxDBClient influxDbClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point influxDbSensor(SENSOR_NAME);

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
    String getLastErrorMessage();
private:
    InfluxDBClient *influxDbClient;
    String lastErrorMessage;
    String sensorName;
    bool influxDbClientInitialised;
};

#endif
