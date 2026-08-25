/* 
  water_level_sensor_calibration.ino
  =====================================
  Reads the distance from the water tank.
  The HC-SR04 sensor sends a trigger signal and waits for an echo signal. The
  distance between the sensor and the water level is calculated based on the
  time between the trigger and the echo signals. Indeed, as the signal will 
  travel at the sound speed (340 m/s) and cover twice the distance, the
  distance between the sensor and the water level in cm is equal to:  
    distance = travel_time * 340 * 1000000 / 100 / 2
  As this distance corresponds to the distance between the sensor and the
  water level, the water level actually depends on the water tank size and is
  calculated by:
    water_level = water_tank_height - distance
  The user can use the push button to activate the pump in order to vary the
  water level and ensure the HC-SR04 sensor accurately measures the change.

  Hardware Connections :
  Refer to the water_level.png image that can be found in the parent folder folder.
  
  Notes:  
  - The DEBUG_MODE constant allows the user to see debug traces. To do so the
    user must uncomment the "//#define DEBUG_MODE" line below in the
    "Constant definitions" section and open the serial port at 9600 baud in
    the Arduino IDE (Tools > Open Serial Monitor).
  - Instead of using an external pulldown resistor on the pin D4, it is possible
    to use the internal pull down resistor by writing
    "pinMode(PIN_BUTTON_PUMP, INPUT_PULLDOWN)" in the setup function.

*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <string.h>


/*--------------------------------
 * Constant definitions
 *--------------------------------*/
// If defined, then the D_TRACE() macro will print useful information on the
// serial port for debugging purpose
#define DEBUG_MODE

// GPIO output pins
#define PIN_RELAY_PUMP  D2
#define PIN_TRIG        D6
// GPIO input pins
#define PIN_ECHO        D8
#define PIN_BUTTON_PUMP D4

// Miscellaneous constants
#define WATER_TANK_HEIGHT_CM  40


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
  Serial.print(F(": "));        \
  Serial.println(s);            \
} while(0)
#else
#define D_TRACE(s)
#endif


/*--------------------------------
 * Global variable definitions
 *--------------------------------*/
volatile uint32_t lastEdgeTime = 0;
// 50 ms seems reliable
volatile uint32_t debounceDelay = 50;


/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that will be called only once before loop(). Contains the serial
/// port and the GPIOs initialization.
///
/// parameters: none
///
/// returns: void
///
void setup() {
  // Set up the GPIOs
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_BUTTON_PUMP, INPUT);
  pinMode(PIN_RELAY_PUMP, OUTPUT);

  //attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_PUMP), handleButton, CHANGE);

  // The relay is active low. Deactivate it by default.
  digitalWrite(PIN_RELAY_PUMP, HIGH);

  // Set up the serial port used to display the value on the screen
  Serial.begin(9600);
  // Wait for the serial port initialization
  delay(2000);

  D_TRACE("setup() done.");
}

/// loop()
///
/// Function that will be looped over indefinitely.
/// Used to calibrate the HC-SR04 sensor.
/// Sends an trigger signal to the HC-SR04 sensor and read when a pulse is
/// received on the echo. Then display the echo duration and the calculated
/// distance.
///
/// parameters: none
///
/// returns: void
///
void loop() {
  uint32_t duration = 0;
  uint32_t distance = 0;

  // Initialize the Trigger pin
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(1);
  // Send the trigger signal
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  //D_TRACE("TRIG signal sent.");

  // Measure the time (in us) before an echo is received
  // TODO[CS]: Add a timeout (check arduino docs for pulseIn)
  duration = pulseIn(PIN_ECHO, HIGH); 
  
  // Compute the distance from the duration:
  //  - Ultrasonic speed is 340 m/s = 0.034 cm/us
  //  - The pulse has travelled twice the distance
  if (!duration) {
    // Did not detect an echo signal. Consider the water level to be low so
    // the user will check what might be wrong
    Serial.println("pulseIn() returned 0. Something went wrong.");
  } else {
    duration /= 2; // Only keep one way
    distance = duration * 0.034; // distance in cm
  }

  // Send the value through the serial port
  Serial.println("duration (us, one way): " + String(duration));
  Serial.println("distance (cm): " + String(distance));
  delay(1000);
}

/// handleButton()
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
  uint32_t currTime = millis();
  byte pushButtState;
  D_TRACE("Button changed -" + String(currTime) + " ms-");

  // Need to debounce the push button. As the input may see erratic
  // rising/falling edges after a push on the button, the code will wait a
  // defined delay after the first edge detected.
  if (currTime - lastEdgeTime > debounceDelay) {
    lastEdgeTime = currTime;
    pushButtState = digitalRead(PIN_BUTTON_PUMP);

    // The relay is active low and the push button is high when pushed so it is
    // the opposite.
    if (pushButtState == HIGH) {
      D_TRACE("Relay activated.");
    } else {
      D_TRACE("Relay deactivated.");
    }
    digitalWrite(PIN_RELAY_PUMP, !pushButtState);
  }
}
