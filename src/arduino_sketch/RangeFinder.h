#pragma once

#define ULTRASONIC_PING 0
#define ULTRASONIC_HC_SR04 1

class RangeFinder {
private:
    const uint8_t* m_sensors;
    uint8_t  m_sensor_count;
    uint8_t  m_sensor_type;

    double* m_ranges;
private:
    double usToCm(unsigned long microseconds);
public:
    RangeFinder() = default;
    ~RangeFinder();
    RangeFinder(const uint8_t* sensors, const uint8_t count, const uint8_t sensorType);

    void   init(const uint8_t* sensors, const uint8_t count, const uint8_t sensorType);
    void   update(); // updates all the distances of all the sensors
    double getDistance(const uint8_t sensor); // returns distance in cm
};