#ifndef _TEMP_DS18B20_H_
#define _TEMP_DS18B20_H_

#include <OneWire.h>

class TempDS18B20 {
public:
    TempDS18B20(int oneWirePin);
    float getTemperatureCelsius();
private:
    byte temperatureSensorAddress[8];
    OneWire oneWire;
    bool setTemperatureSensorAddress();
};

#endif
