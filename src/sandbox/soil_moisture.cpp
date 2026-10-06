/* 
  soil_moisture.cpp
  =====================================
  Code used to test and play with the soil moisture sensor. 
  It consists in a small application that reads the soil moisture sensor value,
  map it to a percentage, and display both the raw value and the percentage on
  the serial port.
  The raw soil moisture sensor value is received on the analog pin A0.
  The data is sent over the serial port at 9600 baud/s.
  
  Hardware Connections :
    Refer to the soil_moisture.png image that can be found in the assets folder.

  Notes:   
    - As references, during testing the sensor returned the following values:
      § dipped into water: 1120
      § into a wet soil: 1200
      § into a fine soil: 1900
      § into a very dry soil: 3280
      § dry out in the air: 3300
*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Arduino.h>
#include "config.h"
#include "macros.h"

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
 //--- Input pin ---//
#define PIN_SENSOR A0

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that will be called only once before loop(). Contains the serial
/// port setup.
///
/// parameters: none
///
/// returns: void
///
void setup() {
  // Set up the serial port used to display the value on the screen
  Serial.begin(9600);
  // Wait for the serial port to open
  delay(2000);
}

/// loop()
///
/// Function that will be looped over indefinitely. All the work is done in
/// interrupt so the function is void.
///
/// parameters: none
///
/// returns: void
///
void loop() {
  int val, perc;
  // Read the value on the analog pin 0
  val = analogRead(PIN_SENSOR);
  perc = map(val, 1100, 3500, 0, 100);
  // Send the value through the serial port
  D_TRACE("Sensor value: " + String(val));
  D_TRACE("Expressed in percentage: " + String(perc));
  delay(1000);
}
