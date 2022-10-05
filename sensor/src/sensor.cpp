#include <OneWire.h>
#include <ArduinoOTA.h>

#include "../../include/settings.cpp"
#include "../include/sensor_settings.cpp"

#ifdef INFLUX_DB
  #include <ESP8266WiFiMulti.h>
  #include "../../lib/influxDbTalker.h"
  InfluxDbTalker *influxDbTalker;
#endif

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>

#include "../../lib/tempDs18b20.cpp"

// DS18B20 pin
#define ONEWIRE_PIN D7
TempDS18B20 temperatureSensor(ONEWIRE_PIN);

// UDP port to send broadcast (both from and to)
#define UDP_PORT 3490
WiFiUDP UDP;
IPAddress broadcastAddress;

#define LED_PIN LED_BUILTIN

IPAddress getBroadcastAddress() {
  IPAddress myAddress = WiFi.localIP();

  myAddress[3] = 255;

  return myAddress;
}

void sendMessage(float temperature) {
  String message = String(SENSOR_NAME) + ":" + String(temperature);
  Serial.printf("Sending message: '%s'\n", message.c_str());

  char messageStr[message.length()+1];
  strcpy(messageStr, message.c_str());

  UDP.beginPacket(broadcastAddress, UDP_PORT);
  UDP.write(messageStr);
  UDP.endPacket();
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  Serial.printf("Connecting to WiFi (%s): ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    digitalWrite(LED_PIN, LOW);
    Serial.print(".");
    delay(100);
    digitalWrite(LED_PIN, HIGH);
  }

  UDP.begin(UDP_PORT);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  broadcastAddress = getBroadcastAddress();
  Serial.print("Broadcast address: ");
  Serial.println(broadcastAddress);

#ifdef INFLUX_DB
  Serial.print("Initialising InfluxDbTalker...");
  influxDbTalker = new InfluxDbTalker(SENSOR_NAME, INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG, INFLUXDB_BUCKET);
  influxDbTalker->begin();
  Serial.println("OK");
#endif

  if (MDNS.begin(SENSOR_NAME)) {
    Serial.println("Started mDNS");
  }
  else {
    Serial.println("Failed to start mDNS");
  }

  ArduinoOTA.begin();
}

// the loop function runs over and over again forever
unsigned int loopCounter = 0;

void loop() {
  MDNS.update();
  ArduinoOTA.handle();
  
  // Reads temperature
  const float tempCelsius = temperatureSensor.getTemperatureCelsius();

  sendMessage(tempCelsius);

  digitalWrite(LED_PIN, LOW);
  delay(1000);

  digitalWrite(LED_PIN, HIGH);
  delay(1000);

#ifdef INFLUX_DB
  if (!influxDbTalker->report("temp", tempCelsius)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(influxDbTalker->getLastErrorMessage());
  }
#endif

  loopCounter++;

  if (DEEP_SLEEP_MICROSECONDS > 0 && loopCounter > 3) {
    Serial.println("Going to deep sleep...");
    ESP.deepSleep(DEEP_SLEEP_MICROSECONDS);
  }
}
