#include <OneWire.h>

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

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

WiFiConnectionHandler ArduinoIoTPreferredConnection(WIFI_SSID, WIFI_PASS);
float tempIndoor;
float tempOutdoor;
bool forceHeatingOn;
bool forceHeatingOff;

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

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);

  setupArduinoCloud();

  tempSensorListener.begin();
}

int forceHeatingAlwaysOn = 0;
int forceHeatingAlwaysOff = 0;

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
  Serial.print("Updating ArduinoCloud...");
  ArduinoCloud.update();
  Serial.println("Updated");
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
}
