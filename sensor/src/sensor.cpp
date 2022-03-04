#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "settings.cpp"

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C

// UDP port to send broadcast (both from and to)
#define UDP_PORT 3490
WiFiUDP UDP;
IPAddress broadcastAddress;

#define LED_PIN BUILTIN_LED

IPAddress getBroadcastAddress() {
  IPAddress myAddress = WiFi.localIP();

  myAddress[3] = 255;

  return myAddress;
}

void sendMessage(double temperature) {
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
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  while(!Serial);
  
  // default settings
  unsigned status;
  status = bme.begin();
  if (!status) {
      while (1) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");

        digitalWrite(BUILTIN_LED, LOW);
        delay(1000);
        digitalWrite(BUILTIN_LED, HIGH);
        delay(1000);
      }
  }

  Serial.printf("Connecting to WiFi (%s): ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    digitalWrite(BUILTIN_LED, LOW);
    Serial.print(".");
    delay(100);
    digitalWrite(BUILTIN_LED, HIGH);
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
}

// the loop function runs over and over again forever
void loop() {
  // Reads temperature
  const double tempCelsius = bme.readTemperature();

  sendMessage(tempCelsius);

  digitalWrite(BUILTIN_LED, LOW);
  delay(1000);

  digitalWrite(BUILTIN_LED, HIGH);
  delay(1000);
}
