/* 
  luminosity_sensor_calibration.ino
  =====================================
  Code used to test and calibrate the luminosity sensor. 
  It simply reads the value on a voltage divider built around a photoresistor.
  The value is read on analog pin A0 and is then sent over the serial port
  @9600 baud.
  To observe the measured value, the user must open the serial port in the
  Arduino IDE (Tools > Open Serial Monitor).
  
  Hardware Connections :
  Refer to the luminosity_sensor_calibration.png image that can be found in
  the parent folder.

  Notes:  
  - The DEBUG_MODE constant allows the user to see debug traces. To do so the
    user must uncomment the "//#define DEBUG_MODE" line below in the
    "Constant definitions" section and open the serial port at 9600 baud in
    the Arduino IDE (Tools > Open Serial Monitor).

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


/*--------------------------------
 * Constant definitions
 *--------------------------------*/
// If defined, then the D_TRACE() macro will print useful information on the
// serial port for debugging purpose
//#define DEBUG_MODE

//--- Output pins ---//
#define PIN_LIGHT_SENSOR      A2

//--- Miscelleanous constants ---//
// Since the relation between the luminosity and the value returned by the
// sensor is not linear, a few thresholds are defined rather than taking the
// value as is and map it to a percentage like for the moisture sensor.
#define LIGHT_THRESHOLD_LOW         1500
#define LIGHT_THRESHOLD_GREAT       4000

/*--------------------------------
 * Macro definitions
 *--------------------------------*/
// Just a tweak to only print the relative path to the filename in D_TRACE
#define __FILENAME__ (strrchr(__FILE__, '\\') ? \
  strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifdef DEBUG_MODE
#define D_TRACE(s) do {         \
  Serial.print(F("[DEBUG] "));  \
  Serial.print(__FILENAME__);   \
  Serial.print(F("@"));         \
  Serial.print(__LINE__);       \
  Serial.print(F(": {"));       \
  Serial.print(__func__);       \
  Serial.print(F("} "));        \
  Serial.println(s);            \
} while(0)
#else
#define D_TRACE(s)
#endif // DEBUG_MODE


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
  val = analogRead(PIN_LIGHT_SENSOR);
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
