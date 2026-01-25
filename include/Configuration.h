#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>

bool prepareFS();
bool loadConfig();
bool saveConfig();
void resetConfig();

#endif
