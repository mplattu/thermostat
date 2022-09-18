#include <OneWire.h>

#include <ESP8266WiFi.h>

#include "../../include/settings.cpp"
#include "../../lib/tempDs18b20.cpp"
#include "../lib/tempSensorListener.cpp"

// DS18B20 pin
#define ONEWIRE_PIN D7
TempDS18B20 outdoorTemperatureSensor(ONEWIRE_PIN);

// UDP port to send broadcast (both from and to)
#define UDP_PORT 3490
TempSensorListener tempSensorListener(UDP_PORT);

#define LED_PIN LED_BUILTIN

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
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

  tempSensorListener.begin();

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// the loop function runs over and over again forever
void loop() {
  // Reads temperature
  const float outdoorTemperature = outdoorTemperatureSensor.getTemperatureCelsius();
  Serial.print("Outdoor temperature: ");
  Serial.println(outdoorTemperature);

  tempSensorListener.readMessages();
  const float averageIndoorTemperature = tempSensorListener.getAverageTemperature();
  Serial.print("Average indoor temperature: ");
  Serial.println(averageIndoorTemperature);

  if (outdoorTemperature + INDOOR_TEMP_TARGET < averageIndoorTemperature) {
    Serial.println("Turn heating off");
  } else {
    Serial.println("Turn heating on");
  }

  digitalWrite(LED_PIN, LOW);
  delay(100);

  digitalWrite(LED_PIN, HIGH);
  delay(100);
}
