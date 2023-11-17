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

void ledOn() {
#ifdef LED_BLINK
  digitalWrite(LED_PIN, LOW);
#else
  digitalWrite(LED_PIN, HIGH);
#endif
}

void ledOnAlways() {
  digitalWrite(LED_PIN, LOW);
}

void ledOff() {
  digitalWrite(LED_PIN, HIGH);
}

void showError(const char * errorMessage, int errorCode) {
  for (int signalCounter = 0; signalCounter < 10; signalCounter++) {
    Serial.print("Error: ");
    Serial.print(errorMessage);

    ledOnAlways();
    delay(5000);
    ledOff();
    delay(1000);

    for (int errorCodeCount = 0; errorCodeCount <= errorCode; errorCodeCount++) {
      ledOnAlways();
      delay(500);
      ledOff();
      delay(500);
    }

    Serial.println("");
  }
}

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

  // Wait 60 seconds for a connection
  unsigned int secondsSinceBoot = millis() / 1000;
  unsigned int secondsSinceBootToReboot = secondsSinceBoot + 60;

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    ledOn();
    Serial.print(".");
    delay(100);
    ledOff();
    secondsSinceBoot = millis() / 1000;
    if (secondsSinceBoot > secondsSinceBootToReboot) {
      showError("No WiFi found", 1);
      ESP.restart();
    }
  }

  UDP.begin(UDP_PORT);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

#ifdef TZ_INFO
 timeSync(TZ_INFO, "pool.ntp.org");
#endif

  broadcastAddress = getBroadcastAddress();
  Serial.print("Broadcast address: ");
  Serial.println(broadcastAddress);

#ifdef INFLUX_DB
  Serial.print("Initialising InfluxDbTalker...");
  influxDbTalker = new InfluxDbTalker(SENSOR_NAME, INFLUXDB_URL, INFLUXDB_TOKEN, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_CERT_SHA1_FINGERPRINT);
  influxDbTalker->begin();
  Serial.println("OK");
#endif

  if (MDNS.begin(SENSOR_NAME)) {
    Serial.println("Started mDNS");
  }
  else {
    showError("Failed to start mDNS", 2);
  }

  ArduinoOTA.begin();
}

// the loop function runs over and over again forever
unsigned int loopCounter = 0;

void loop() {
  MDNS.update();
  ArduinoOTA.handle();
  
  if (WiFi.status() != WL_CONNECTED) {
    showError("WiFi connection lost, rebooting", 3);
    ESP.restart();
  }

  // Reads temperature
  const float tempCelsius = temperatureSensor.getTemperatureCelsius();

  sendMessage(tempCelsius);

  ledOn();
  delay(1000);
  ledOff();
  delay(1000);

#ifdef INFLUX_DB
  if (!influxDbTalker->report("temp", tempCelsius)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(influxDbTalker->getLastErrorMessage());
    showError("InfluxDB write error when reporting temp", 4);
  }
  if (!influxDbTalker->report("ipv4", WiFi.localIP().toString())) {
    Serial.print("InfluxDB write failed (reporting ipv4): ");
    Serial.println(influxDbTalker->getLastErrorMessage());
    showError("InfluxDB write error when reporting IPv4", 5);
  }
#endif

  loopCounter++;

  if (DEEP_SLEEP_MICROSECONDS > 0 && loopCounter > 3) {
    Serial.println("Going to deep sleep...");
    ESP.deepSleep(DEEP_SLEEP_MICROSECONDS);
  }
}
