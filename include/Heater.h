#ifndef HEATER_H
#define HEATER_H

#include <Arduino.h>

void setupHeater();
void updateHeater();
void setHeatPowerPercentage(float power);
float getHeatCycles();
void _turnHeatElementOnOff(boolean on);

#endif
