/* 
  sensors.cpp
  =====================================
  Source code of the Sensors module of the project.
  The project uses 3 sensors:
    - a soil moisture sensor
    - a luminosity sensor
    - a distance sensor to measure the water level

  Read the file header in sensors.h for more details.
 */

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Arduino.h>
#include "config.h"
#include "macros.h"
#include "sensors.h"

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// readMoistureSensor()
///
/// Function to read the value returned by the soil moisture sensor.
/// Read the function declaration in sensors.h for more details.
///
/// parameters: none
///
/// returns: (uint16_t) raw soil moisture value read from the sensor
///
uint16_t readMoistureSensor() {
  uint16_t moistVal = analogRead(PIN_MOIST_SENSOR);

  D_TRACE("Measured moisture sensor value: " + String(moistVal));

  return moistVal;
}

/// readLuminositySensor()
///
/// Function to read the value returned by the luminosity sensor.
/// Read the function declaration in sensors.h for more details.
///
/// parameters: none
///
/// returns: (uint16_t) raw luminosity value read from the sensor
///
uint16_t readLuminositySensor() {
  uint16_t lumVal = analogRead(PIN_LUM_SENSOR);

  D_TRACE("Measured luminosity sensor value: " + String(lumVal));

  return lumVal;
}

/// isWaterLevelLow()
///
/// Function to determine if the water level in the tank is low or not.
/// Read the function declaration in sensors.h for more details.
///
/// parameters: none
///
/// returns: (bool) true if the water level is lower than 1 cm, false otherwise
///
bool isWaterLevelLow() {
  uint32_t duration = 0;
  uint32_t distance = 0;
  uint32_t waterLevel = 0;

  // Initialize the Trigger pin
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(1);
  // Send the trigger signal
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  D_TRACE("TRIG signal sent.");

  // Measure the time (in us) before an echo is received
  // TODO[CS]: Add a timeout (check arduino docs for pulseIn)
  duration = pulseIn(PIN_ECHO, HIGH); 
  
  // Compute the distance from the duration:
  //  - Ultrasonic speed is 340 m/s = 0.034 cm/us
  //  - The pulse has travelled twice the distance
  if (!duration) {
    // Did not detect an echo signal. Consider the water level to be low so
    // the user will check what might be wrong
    D_TRACE("pulseIn() returned 0. Something went wrong.");
  } else {
    // Distance between the sensor and the water
    distance = duration / 2 * 0.34;
    // Infer the water level from the distance between the sensor and the water
    // (add 10 mm as a margin)
    waterLevel = WATER_SENSOR_DISTANCE + 10 - distance;
  }

  // Send the value through the serial port
  D_TRACE("distance (mm): " + String(distance) + "\twater level (mm): " + String(waterLevel));

  return (waterLevel < 10);
}