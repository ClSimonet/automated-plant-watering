/* 
  Moist_sensor_calibration.ino
  =====================================
   Simply reads the values from the capacitive moisture sensor.
   It is intended to determine the value returned by the sensor when put in a
   dry soil and humid soil. That way, these values can be used as threshold
   later to consider if the soil is dry or not.
   The analog pin A0 receives the value from the sensor. It is then sent over
   the serial port @9600 baud.
   To observe the measured value, the user must open the serial port in the
   Arduino IDE (Tools > Open Serial Monitor).

  Hardware Connections :
  Refer to the moist_sensor_calibration.png image that can be found in the
  same folder.
  
  Notes:  
  - during testing, the moisture sensor returned a value of ... for a soil
    considered fine, a value of 2800 for a soil considered dry, a value of ...
    for a soil considered very dry, and a value of ... for a soil considered
    very wet. 
*/

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
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
  Serial.println("Sensor value: " + String(val));
  Serial.println("Expressed in percentage: " + String(perc));
  delay(1000);
}
