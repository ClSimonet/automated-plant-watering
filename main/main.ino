/* 
  main.ino
  =====================================
  Control the plant watering system. The system is composed of 4 units:
    - The Arduino board
    - The water pump motor controlled by a relay module
    - The soil moisture sensor
    - A switch to override the sensor check and force the activation of the
      pump
  
  Hardware Connections :
  Refer to the main.png image that can be found in the same folder.

  Note:  
  - the pump is only activated for 5s when the soil is considered dry. It may
    not be enough time to humidify the soil correctly. It shouldn't be too much
    impactful because then the soil would be watered again on the next polling
    time until it is considered sufficiently watered. But this 5s watering time
    might be increased in the future if needed.
  
  - the sensor value could be refined to be more comprehensive. The map()
    function would be really useful for this. It will be required if a screen
    is added to indicate the moisture pourcentage for example.
  
  - during testing, the moisture sensor returned values never less than 1100
    (dipped into water) and never more than 3350 (out in the air).
  
  - during testing, the moisture sensor returned a value of ... for a soil
    considered fine, a value of 2800 for a soil considered dry, a value of ...
    for a soil considered very dry, and a value of ... for a soil considered
    very wet. 

  - The DEBUG_MODE constant allows the user to see debug traces. To do so the
    user must uncomment the "//#define DEBUG_MODE" line below in the
    "Constant definitions" section and open the serial port at 9600 baud in
    the Arduino IDE (Tools > Open Serial Monitor).
*/

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
//#define DEBUG_MODE

/* Input pins */
#define PIN_PUMP_BUTTON 4
#define PIN_MOIST_SENSOR A0

/* Output pins */
// The yellow LED indicates the soil is dry
#define PIN_LED_Y 6
// The green LED indicates the pump is on
#define PIN_LED_G 8
// The relay directly commands the water pump
#define PIN_RELAY 12

/* Constants */
// Represents the dryness of the soil. A dryness greater than
// BASE_DRY_THRESHOLD indicates the soil is dry. After calibration of the
// sensor, the threshold is considered to be 1900 in absolute value.
#define BASE_DRY_THRESHOLD 1900
// Period (in minutes) at which the moisture sensor will be read
#define POLLING_PERIOD_M 30
// Time in ms to wait to avoid bounces on the push button
#define DEBOUNCE_DELAY 50

/* Global variables */
// Used to debounce the push buttons
volatile unsigned short lastEdgeTime = 0;

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
  // Set up the GPIOs
  pinMode(PIN_PUMP_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_MOIST_SENSOR, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_Y, OUTPUT);

  // Turn off the LEDs at initialization
  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_Y, LOW);

  // Set up the interrupt
  attachInterrupt(digitalPinToInterrupt(PIN_PUMP_BUTTON), handlePumpButton, CHANGE);

#ifdef DEBUG_MODE
  // Set up the serial port used to display the value on the screen
  Serial.begin(9600);
#endif
}

/// loop()
///
/// Function that will be looped over indefinitely. Polls the moisture sensor
/// value periodically and activate the pump for 5s if the soil is dry, then
/// deactivate the pump.
///
/// parameters: none
///
/// returns: void
///
void loop() {
  if (isSoilDry()) {
    // The soil is dry! Turn on the pump for 5s and light on the green LED to
    // signal it's currently watering.
#ifdef DEBUG_MODE
    Serial.println("loop(): The soil is dry!");
#endif
    digitalWrite(PIN_LED_Y, HIGH);
    actionPump(true);
    digitalWrite(PIN_LED_G, HIGH);
    delay(5000);
    actionPump(false);
    digitalWrite(PIN_LED_G, LOW);
  } else {
    digitalWrite(PIN_LED_Y, LOW);
  }
  // The soil moisture will be polled next time based on the value defined in
   // the constant definitions section above. delay() takes the time in ms.
#ifdef DEBUG_MODE
  // While testing poll every 5s
  delay(5000);
#else
  delay(POLLING_PERIOD_M * 3600);
#endif
}

/// isSoilDry()
///
/// Function to determine if the soil is dry or not. It will read the value
/// returned by the moisture sensor and if the value is higher than the defined
/// threshold then it means the soil is dry.
///
/// parameters: none
///
/// returns: true if the soil is dry, false otherwise
///
bool isSoilDry() {
  int valSensor = analogRead(PIN_MOIST_SENSOR);
#ifdef DEBUG_MODE
  Serial.println("isSoilDry(): valSensor = " + String(valSensor));
#endif /* DEBUG_MODE */

  return valSensor > BASE_DRY_THRESHOLD;
}

/// actionPump()
///
/// Function to activate or deactivate the relay that controls the water pump.
///
/// parameters:
///   - toActivate : indicate if the caller intends to activate or deactivate
///       the pump. `true` means to activate, `false` means to deactivate.
///
/// returns: void
///
void actionPump(bool toActivate) {
  if (toActivate) {
    // The relay is active low so `LOW` means ON.
    digitalWrite(PIN_RELAY, LOW);
#ifdef DEBUG_MODE
    Serial.println("actionPump(): Pump activated!");
#endif /* DEBUG_MODE */
  } else {
    digitalWrite(PIN_RELAY, HIGH);
#ifdef DEBUG_MODE
    Serial.println("actionPump(): Pump deactivated!");
#endif /* DEBUG_MODE */
  }
}

/// handlePumpButton()
///
/// ISR that is called when the push button that manually controls the pump is
/// pressed or released. It actually activate or deactivate the relay that
/// controls the water pump.
///
/// Note: It does maybe too much work for an ISR but it is fine for the moment.
///
/// parameters: none
///
/// returns: void
///
void handlePumpButton() {
  // millis() is awkward in ISR but it's fine here since it's done as the very
  // first instruction.
  unsigned long currTime = millis();
  // Just a default value
  bool pushButtState = LOW;

#ifdef DEBUG_MODE
  Serial.println("handlePumpButton(): In the ISR!");
#endif /* DEBUG_MODE */

  // Need to debounce the push button. As the input may see erratic
  // rising/falling edges after a push on the button, the code will wait a
  // defined delay after the first edge detected.
  if (currTime - lastEdgeTime > DEBOUNCE_DELAY) {
    lastEdgeTime = currTime;
    pushButtState = digitalRead(PIN_PUMP_BUTTON);

    // The relay is active low and the push button is high when pushed so it is
    // the opposite.
    digitalWrite(PIN_RELAY, !pushButtState);
    // Also light on a LED to give a feedback to the user.
    digitalWrite(PIN_LED_G, pushButtState);
#ifdef DEBUG_MODE
    Serial.println("handlePumpButton(): User force the pump!");
#endif /* DEBUG_MODE */
  }
}
