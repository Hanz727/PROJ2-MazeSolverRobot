#include "RangeFinder.h"

const uint8_t pins[3] = {6,7,8};

RangeFinder rangeFinder;

void setup() {
  // put your setup code here, to run once:
  rangeFinder.init(pins, 3, ULTRASONIC_PING);
}

void loop() {
  // put your main code here, to run repeatedly:
  rangeFinder.update();

}
