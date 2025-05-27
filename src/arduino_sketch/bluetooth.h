#include "Arduino.h"
#include <avr/wdt.h>
#include "MazeSolver.h"

#define Bluetooth Serial2

void handleBluetoothCmds(int8_t& width, int8_t& height, bool& start, void (*forward)(), void (*backward)(), void (*left)(), void (*right)(), vec2<int8_t>& startPos, vec2<int8_t>& endPos);
