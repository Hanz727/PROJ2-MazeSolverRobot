#include "Arduino.h"
#include <avr/wdt.h>

#define Bluetooth Serial2

void handleBluetoothCmds(int8_t& width, int8_t& height, bool& start, void (*forward)(), void (*backward)(), void (*left)(), void (*right)()); 
