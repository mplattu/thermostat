#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#else
#define DEBUG_MSG(...)
#endif

#include <OneWire.h>
#include <ArduinoOTA.h>

#include "../lib/relayController.h"

#include "../../include/settings.cpp"
#include "../include/server_settings.cpp"

#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

#ifdef INFLUX_DB
  #include "../../lib/influxDbTalker.h"
  InfluxDbTalker influxDbTalker(SERVER_NAME, INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_CERT_SHA1_FINGERPRINT);
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

#include <ESP8266mDNS.h>

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

void dumpMemoryStatus() {
  Serial.printf_P(PSTR("Free_heap %d, max_alloc_heap %d, heap_fragmentation  %d\n"), ESP.getFreeHeap(), ESP.getMaxFreeBlockSize(), ESP.getHeapFragmentation());
}

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

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.hostname(SERVER_NAME);

#ifdef ARDUINO_IOT_CLOUD
  setupArduinoCloud();
#endif

  Serial.print(F("Connecting to WiFi..."));
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(F("Connected"));

#ifdef TZ_INFO
  timeSync(TZ_INFO, "pool.ntp.org");
#endif

  forceHeatingOff = false;
  forceHeatingOn = false;

  dumpMemoryStatus();

#ifdef INFLUX_DB
  Serial.print(F("Initialising InfluxDbTalker..."));
  if (!influxDbTalker.begin()) {
    Serial.println(influxDbTalker.getLastErrorMessage());
    delay(5000);
  }
  Serial.print(F("OK, connected to "));
  Serial.println(influxDbTalker.getServerUrl());
#endif
  
  if (MDNS.begin(SERVER_NAME)) {
    Serial.println(F("Started mDNS"));
  }
  else {
    Serial.println(F("Failed to start mDNS"));
  }

  tempSensorListener.begin();

  relaysSetup();
  Serial.print(F("Initialising relays..."));
  for (int n=0; n<RELAYS_COUNT; n++) {
    if (RELAYS[n].begin()) {
      Serial.print(F("OK "));
    }
    else {
      Serial.print(F("Error "));
    }
  }
  Serial.println(F("done"));
}

void turnHeatingOff(bool forced) {
  Serial.print(F("Turning heating off"));
  if (forced) {
    Serial.print(F(" (forced)"));
  }

  for (int n=0; n<RELAYS_COUNT; n++) {
    if (RELAYS[n].off()) {
      Serial.print(F(" OK"));
    }
    else {
      Serial.print(F(" Error"));
    }
  }

  Serial.println();
}

void turnHeatingOn(bool forced) {
  Serial.print(F("Turning heating on"));
  if (forced) {
    Serial.print(" (forced)");
  }

  for (int n=0; n<RELAYS_COUNT; n++) {
    if (RELAYS[n].on()) {
      Serial.print(F(" OK"));
    }
    else {
      Serial.print(F(" Error"));
    }
  }

  Serial.println();
}

// the loop function runs over and over again forever
void loop() {
  Serial.println(F("-------------Loop starts"));

  MDNS.update();
  ArduinoOTA.handle();

  digitalWrite(LED_PIN, LOW);
#ifdef ARDUINO_IOT_CLOUD
  int freeHeapArduinoBefore = system_get_free_heap_size();
  Serial.print("Updating ArduinoCloud...");
  ArduinoCloud.update();
  Serial.print("Updated ");
  int freeHeapArduinoAfter = system_get_free_heap_size();
#else
  delay(1000);
#endif
  digitalWrite(LED_PIN, HIGH);
 
  // Reads temperature
  tempOutdoor = outdoorTemperatureSensor.getTemperatureCelsius();
  Serial.print(F("Outdoor temperature: "));
  Serial.println(tempOutdoor);

  tempSensorListener.readMessages();
  tempIndoor = tempSensorListener.getAverageTemperature();
  Serial.print(F("Average indoor temperature: "));
  Serial.println(tempIndoor);

  int freeHeapRelaysBefore = system_get_free_heap_size();
  if (forceHeatingOff == 1) {
    turnHeatingOff(true);
  } else if (forceHeatingOn == 1) {
    turnHeatingOn(true);
  } else if (tempOutdoor + INDOOR_TEMP_TARGET < tempIndoor) {
    turnHeatingOff(false);
  } else {
    turnHeatingOff(false);
  }
  int freeHeapRelaysAfter = system_get_free_heap_size();

#ifdef INFLUX_DB
 if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  if (!influxDbTalker.report("temp", tempOutdoor)) {
    Serial.print(F("InfluxDB write failed (reporting temp): "));
    Serial.println(influxDbTalker.getLastErrorMessage());
  }
  if (!influxDbTalker.report("ipv4", WiFi.localIP().toString())) {
    Serial.print(F("InfluxDB write failed (reporting ipv4): "));
    Serial.println(influxDbTalker.getLastErrorMessage());
  }
#endif

#ifdef ARDUINO_IOT_CLOUD
  Serial.print(F("Arduino used memory: "));
  Serial.println(freeHeapArduinoAfter - freeHeapArduinoBefore);
#endif

  Serial.print(F("Relays used memory: "));
  Serial.println(freeHeapRelaysAfter - freeHeapRelaysBefore);

  Serial.print(F("Loop ends: "));
  Serial.println(system_get_free_heap_size());

  delay(1000);
}
