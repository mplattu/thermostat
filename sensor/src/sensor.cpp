#include <OneWire.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include "settings.cpp"

// DS18B20 pin
#define ONEWIRE_PIN D7
OneWire ds(ONEWIRE_PIN);
byte temperatureSensorAddress[8];

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

bool setTemperatureSensorAddress() {
  if ( !ds.search(temperatureSensorAddress)) {
    ds.reset_search();
    delay(250);
    Serial.println("Failed to find DS device");
    return false;
  }

  if (OneWire::crc8(temperatureSensorAddress, 7) != temperatureSensorAddress[7]) {
      Serial.println("CRC is not valid!");
      return false;
  }

  return true;
}

float getTemperatureCelsius() {
  byte i;
  byte data[12];
  float celsius;
  
  ds.reset();
  ds.select(temperatureSensorAddress);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end  
  delay(1000);
  ds.reset();
  ds.select(temperatureSensorAddress);    
  ds.write(0xBE);         // Read Scratchpad
 
  for ( i = 0; i < 9; i++) {           
    data[i] = ds.read();
  }
 
  // Convert the data to actual temperature
  int16_t raw = (data[1] << 8) | data[0];

  // This rule is valid ONLY for DS18B20
  byte cfg = (data[4] & 0x60);
  if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms

  celsius = (float)raw / 16.0;

  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.println(" Celsius");

  return celsius;
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

  UDP.begin(UDP_PORT);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  broadcastAddress = getBroadcastAddress();
  Serial.print("Broadcast address: ");
  Serial.println(broadcastAddress);

  while (!setTemperatureSensorAddress()) {
    Serial.println("Failed to get OneWire sensor address");
    delay(1000);
  }
}

// the loop function runs over and over again forever
void loop() {
  // Reads temperature
  const float tempCelsius = getTemperatureCelsius();

  sendMessage(tempCelsius);

  digitalWrite(LED_PIN, LOW);
  delay(1000);

  digitalWrite(LED_PIN, HIGH);
  delay(1000);
}
