#include "tempDs18b20.h"

TempDS18B20::TempDS18B20(int oneWirePin) {
    this->oneWire = OneWire(oneWirePin);
    while (! setTemperatureSensorAddress()) {
        delay(500);
    }
}

bool TempDS18B20::setTemperatureSensorAddress() {
  if ( !this->oneWire.search(this->temperatureSensorAddress)) {
    this->oneWire.reset_search();
    delay(250);
    Serial.println("Failed to find DS18B20 device");
    return false;
  }

  if (OneWire::crc8(this->temperatureSensorAddress, 7) != this->temperatureSensorAddress[7]) {
      Serial.println("CRC is not valid!");
      return false;
  }

  return true;
}

float TempDS18B20::getTemperatureCelsius() {
  byte i;
  byte data[12];
  float celsius;
  
  this->oneWire.reset();
  this->oneWire.select(this->temperatureSensorAddress);
  this->oneWire.write(0x44, 1);        // start conversion, with parasite power on at the end  
  delay(1000);
  this->oneWire.reset();
  this->oneWire.select(this->temperatureSensorAddress);    
  this->oneWire.write(0xBE);         // Read Scratchpad
 
  for ( i = 0; i < 9; i++) {           
    data[i] = this->oneWire.read();
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

