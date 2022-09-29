#include <OneWire.h>

#include "../../include/settings.cpp"
#include "../include/server_settings.cpp"

#ifdef INFLUX_DB
  #include <InfluxDbClient.h>
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
#endif

#ifdef ARDUINO_IOT_CLOUD
  #include <ArduinoIoTCloud.h>
  #include <Arduino_ConnectionHandler.h>
#endif

#ifndef INFLUX_DB
  #ifndef ARDUINO_IOT_CLOUD
    #include <ESP8266WiFiMulti.h>
    ESP8266WiFiMulti wifiMulti;
  #endif
#endif

#include "../../lib/tempDs18b20.cpp"
#include "../lib/tempSensorListener.cpp"

// DS18B20 pin
#define ONEWIRE_PIN D7
TempDS18B20 outdoorTemperatureSensor(ONEWIRE_PIN);

// UDP port to send broadcast (both from and to)
#define UDP_PORT 3490
TempSensorListener tempSensorListener(UDP_PORT);

#define LED_PIN LED_BUILTIN

float tempIndoor;
float tempOutdoor;
bool forceHeatingOn;
bool forceHeatingOff;

#ifdef ARDUINO_IOT_CLOUD
WiFiConnectionHandler ArduinoIoTPreferredConnection(WIFI_SSID, WIFI_PASS);

void setupArduinoCloud() {
  Serial.print("Setting board id and secret key...");
  ArduinoCloud.setBoardId(ARDUINO_DEVICE_ID);
  ArduinoCloud.setSecretDeviceKey(ARDUINO_DEVICE_SECRET_KEY);
  Serial.println("Set");

  Serial.print("Setting properties...");
  ArduinoCloud.addProperty(forceHeatingOn, READWRITE, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(forceHeatingOff, READWRITE, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(tempIndoor, READ, 10 * SECONDS, NULL);
  ArduinoCloud.addProperty(tempOutdoor, READ, 10 * SECONDS, NULL);
  Serial.println("Set");

  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  //ArduinoCloud.printDebugInfo();
}
#endif

#ifdef INFLUX_DB
InfluxDBClient influxDbClient(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point influxDbSensor(SERVER_NAME);
#endif

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

#ifdef ARDUINO_IOT_CLOUD
  setupArduinoCloud();
#else
  Serial.println("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected");

  forceHeatingOff = false;
  forceHeatingOn = false;
#endif

#ifdef INFLUX_DB
  if (influxDbClient.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(influxDbClient.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(influxDbClient.getLastErrorMessage());
  }
#endif

  tempSensorListener.begin();
}

void turnHeatingOff(bool forced) {
  Serial.print("Turning heating off");
  if (forced) {
    Serial.print(" (forced)");
  }
  Serial.println();
}

void turnHeatingOn(bool forced) {
  Serial.print("Turning heating on");
  if (forced) {
    Serial.print(" (forced)");
  }
  Serial.println();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_PIN, LOW);
#ifdef ARDUINO_IOT_CLOUD
  Serial.print("Updating ArduinoCloud...");
  ArduinoCloud.update();
  Serial.println("Updated");
#else
  delay(1000);
#endif
  digitalWrite(LED_PIN, HIGH);
 
  // Reads temperature
  tempOutdoor = outdoorTemperatureSensor.getTemperatureCelsius();
  Serial.print("Outdoor temperature: ");
  Serial.println(tempOutdoor);

  tempSensorListener.readMessages();
  tempIndoor = tempSensorListener.getAverageTemperature();
  Serial.print("Average indoor temperature: ");
  Serial.println(tempIndoor);

  if (forceHeatingOff == 1) {
    turnHeatingOff(true);
  } else if (forceHeatingOn == 1) {
    turnHeatingOn(true);
  } else if (tempOutdoor + INDOOR_TEMP_TARGET < tempIndoor) {
    turnHeatingOff(false);
  } else {
    turnHeatingOff(false);
  }

#ifdef INFLUX_DB
  influxDbSensor.clearFields();
  influxDbSensor.addField("temp", tempOutdoor);
  influxDbClient.pointToLineProtocol(influxDbSensor);
  if (!influxDbClient.writePoint(influxDbSensor)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(influxDbClient.getLastErrorMessage());
  }
#endif
}
