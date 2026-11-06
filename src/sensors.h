#ifndef SENSORS_H
#define SENSORS_H

#include <Wire.h>

void initSensors();
float getTemperature();
float getHumidity();
int getAirQuality();
bool getDark();
String getTime();
String getDateTime();

#endif
