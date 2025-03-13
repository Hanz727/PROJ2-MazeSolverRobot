#include "Arduino.h"
#include <avr/wdt.h>

#define Bluetooth Serial2

void handleBluetoothCmds(uint8_t& width, uint8_t& height, bool& start); 
