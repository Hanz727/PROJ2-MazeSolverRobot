# RangeFinder usage guide
This module is responsible for interfacing with multiple ultrasonic sensors in an easy way. 

### PING Ultrasonic Sensor
```c
    #include "RangeFinder.h"
    
    const int sensors = {6,7,8}; // PING pins, one pin for each sensor
   
    RangeFinder rangeFinder;
    
    void setup() {
        rangeFinder.init(sensors, 3, ULTRASONIC_PING);
    }
    
    void update() {
        rangeFinder.update(); // updates the ranges for all sensors
        
        // Gets measured distance from sensor on pin 6 in cm
        double dist0 = rangeFinder.getDistance(0);
        
    }
```

### HR-SR04 Ultrasonic Sensor
```c
    #include "RangeFinder.h"
    
    const int sensors = {6,7,8,9,10,11}; // ECHO,TRIG pin pairs, two pins for each sensor
   
    RangeFinder rangeFinder;
    
    void setup() {
        rangeFinder.init(sensors, 3, ULTRASONIC_HC_SR04); // The count is still 3, because we have 3 sensors.
    }
    
    void update() {
        rangeFinder.update(); // updates the ranges for all sensors
        
        // Gets measured distance from sensor on pin 6 in cm
        double dist0 = rangeFinder.getDistance(0);
        
    }

```