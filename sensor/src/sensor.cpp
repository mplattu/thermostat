#include <MAX6675_Thermocouple.h>
#include <Thermocouple.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "settings.cpp"

// MAX6675 connectors and thermocouple object
#define SCK_PIN D5
#define CS_PIN D7
#define SO_PIN D6
Thermocouple* thermocouple;

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

  thermocouple = new MAX6675_Thermocouple(SCK_PIN, CS_PIN, SO_PIN);

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
  const double tempCelsius = thermocouple->readCelsius();

  sendMessage(tempCelsius);

  digitalWrite(BUILTIN_LED, LOW);
  delay(1000);

  digitalWrite(BUILTIN_LED, HIGH);
  delay(1000);
}
