/* 
  luminosity.cpp
  =====================================
  Code used to test and play with the luminosity sensor. 
  It consists in a small application that reads the luminosity sensor value,
  display it on the serial port, and compare it with predefined thresholds.
  The raw luminosity sensor value reflects the voltage on a voltage divider
  built around a photoresistor. This value is received on the analog pin A2.
  The data is sent over the serial port at 9600 baud/s.
  
  Hardware Connections :
    Refer to the luminosity.png image that can be found in the assets folder.

  Notes:   
    - As references, during testing the sensor returned the following values:
        § 4095 when placed outside directly exposed to the sun
        § 3900 when placed outside in the shades, or during a cloudy day
        § 0 when placed outside at night
        § 0 when placed inside in a dark room
        § 3200 when placed in a room with natural light
        § 3000 when placed in a room with artificial light
*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Arduino.h>
#include "config.h"
#include "macros.h"

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that will be called only once before loop(). 
/// Contains the serial port initialization.
///
/// parameters:
///     none
///
/// returns:
///     void
///
void setup(void) {
  // Set up the serial port used to display debug traces
  Serial.begin(9600);
  // Wait for the serial port initialization
  delay(2000);
  D_TRACE("Debug traces enabled.");
}

/// loop()
///
/// Function that will be looped over indefinitely.
/// Read the luminosity sensor value and display it on the serial port.
///
/// parameters:
///     none
///
/// returns:
///     void
///
void loop(void) {
  uint16_t val;

  // Read the value on the analog pin 0
  val = analogRead(PIN_LUM_SENSOR);
  D_TRACE("Sensor value read.");
  // Send the value through the serial port
  Serial.println("Sensor value: " + String(val));
  if (val > LIGHT_THRESHOLD_GREAT) {
    Serial.println("LIGHTMAXXING!");
  } else if (val > LIGHT_THRESHOLD_LOW) {
    Serial.println("LIGHT OK");
  } else {
    Serial.println("LIGHT LOW...");
  } 
  delay(1000);
}
