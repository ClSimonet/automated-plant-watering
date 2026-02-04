/* 
  moist_sensor_calibration.ino
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
  Refer to the Test_command_relay_with_push_button.png image that can be found in the
  same folder.
  
  Notes:  
  - The DEBUG_MODE constant allows the user to see debug traces. To do so the
    user must uncomment the "//#define DEBUG_MODE" line below in the
    "Constant definitions" section and open the serial port at 9600 baud in
    the Arduino IDE (Tools > Open Serial Monitor).
  - A debounce delay of 50ms seems to be the most reliable.
  - Instead of using an external pulldon resistor on the pin D2, it is possible
    to use the internal pull down resistor by writing
    "pinMode(PIN_PUSH_BUTTON, INPUT)" in the setup function.
*/

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
#define DEBUG

/* Input pins */
#define PIN_PUSH_BUTTON 2

/* Output pins */
#define PIN_RELAY 7

/*--------------------------------
 * Global variable definitions
 *--------------------------------*/
volatile unsigned long lastEdgeTime = 0;
// 50 ms seems reliable
volatile unsigned long debounceDelay = 50;

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that will be called only once before loop(). Contains the board,
/// interrupts and variable setup.
///
/// parameters: none
///
/// returns: void
///
void setup() {
  pinMode(PIN_PUSH_BUTTON, INPUT);
  pinMode(PIN_RELAY, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_PUSH_BUTTON), handleButton, CHANGE);

  Serial.begin(9600);

  // The relay is active low. Deactivate it by default.
  digitalWrite(PIN_RELAY, HIGH);
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
  // everything done in ISR
}

/// handlePumpButton()
///
/// ISR that is called when the push button that manually controls the pump is
/// pressed or released. It actually activates or deactivates the relay that
/// controls the water pump.
///
/// Note: It does maybe too much work for an ISR but it is fine for the moment.
///
/// parameters: none
///
/// returns: void
///
void handleButton() {
  // millis() is awkward in ISR but it's fine here since it's done as the very
  // first instruction.
  unsigned long currTime = millis();
  byte pushButtState;
#ifdef DEBUG
  Serial.println("[handleButton()] Button changed -" + String(currTime) + " ms-");
#endif

  // Need to debounce the push button. As the input may see erratic
  // rising/falling edges after a push on the button, the code will wait a
  // defined delay after the first edge detected.
  if (currTime - lastEdgeTime > debounceDelay) {
    lastEdgeTime = currTime;
    pushButtState = digitalRead(PIN_PUSH_BUTTON);

    // The relay is active low and the push button is high when pushed so it is
    // the opposite.
    digitalWrite(PIN_RELAY, !pushButtState);
#ifdef DEBUG
    Serial.println("[handleButton()] Relay activated/deactivated.");
#endif
  }
}
