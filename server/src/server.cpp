#include <OneWire.h>

#include <ESP8266WiFi.h>

#include "../../include/settings.cpp"
#include "../../lib/tempDs18b20.cpp"
#include "../lib/tempSensorListener.cpp"

#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp8266_SSL.h>

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

  Serial.println("Now starting Blynk");
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  Serial.println("Blynk started");

  tempSensorListener.begin();
}

int forceHeatingAlwaysOn = 0;
int forceHeatingAlwaysOff = 0;

// V1: Heating always on
BLYNK_WRITE(V1) {
  forceHeatingAlwaysOn = param.asInt();
  Serial.println("BLYNK_WRITE V1 CALLED");
}

// V2: Heating always off
BLYNK_WRITE(V2) {
  forceHeatingAlwaysOff = param.asInt();
  Serial.println("BLYNK_WRITE V2 CALLED");
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
  Serial.print("Blynk loop...");
  Blynk.run();
  Serial.println("OK");

  // Reads temperature
  const float outdoorTemperature = outdoorTemperatureSensor.getTemperatureCelsius();
  Serial.print("Outdoor temperature: ");
  Serial.println(outdoorTemperature);

  tempSensorListener.readMessages();
  const float averageIndoorTemperature = tempSensorListener.getAverageTemperature();
  Serial.print("Average indoor temperature: ");
  Serial.println(averageIndoorTemperature);

  Blynk.virtualWrite(V0, averageIndoorTemperature);
  Blynk.virtualWrite(V3, outdoorTemperature);
  
  if (forceHeatingAlwaysOff == 1) {
    turnHeatingOff(true);
  } else if (forceHeatingAlwaysOn == 1) {
    turnHeatingOn(true);
  } else if (outdoorTemperature + INDOOR_TEMP_TARGET < averageIndoorTemperature) {
    turnHeatingOff(false);
  } else {
    turnHeatingOff(false);
  }

  digitalWrite(LED_PIN, LOW);
  delay(500);

  digitalWrite(LED_PIN, HIGH);
  delay(500);
}
