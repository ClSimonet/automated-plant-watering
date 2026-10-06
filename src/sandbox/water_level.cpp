/* 
  water_level.cpp
  =====================================
  Code used to test and play with the water level sensor. 
  It consists in a small application that determines the level of water in the tank from the ultrasonic sensor, and send all the useful information (travel
  time of the sonic impulse, distance between the sensor and the water, water
  level in the tank) over the serial port at 9600 baud/s.
  The HC-SR04 sensor sends a trigger signal and waits for an echo signal. The
  distance between the sensor and the water level is calculated based on the
  time between the trigger and the echo signals. Indeed, as the signal will 
  travel at the sound speed (340 m/s) and cover twice the distance, the
  distance between the sensor and the water level in mm is equal to:  
    distance = travel_time / 2 * 340 * 1000 / 1000000
  As this distance corresponds to the distance between the sensor and the
  water level, the water level actually depends on the water tank size and is
  calculated by:
    water_level = water_tank_height - distance
  The user can use the push button to activate the pump in order to vary the
  water level and ensure the HC-SR04 sensor accurately measures the change.
   
  Hardware Connections :
    Refer to the water_level.png image that can be found in the assets folder.

  Notes:   
    - Instead of using an external pulldown resistor on the pin D4, it is
      possible to use the internal pull down resistor by writing
      "pinMode(PIN_BUTTON_PUMP, INPUT_PULLDOWN)" in the setup function.
*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Arduino.h>
#include "config.h"
#include "macros.h"

/*--------------------------------
 * Global variable definitions
 *--------------------------------*/
volatile uint32_t lastEdgeTime = 0;
// 50 ms seems reliable
volatile uint32_t debounceDelay = 50;

/*--------------------------------
 * Forward declaration
 *--------------------------------*/
void handleButton(void);

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
  pinMode(PIN_PUMP_BUTTON, INPUT);
  pinMode(PIN_RELAY, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_PUMP_BUTTON), handleButton, CHANGE);

  // The relay is active low. Deactivate it by default.
  digitalWrite(PIN_RELAY, HIGH);

  // Set up the serial port used to display the value on the screen
  initLogger(9600);

  D_TRACE("Setup done.");
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
  uint32_t waterLevel = 0;

  // Initialize the Trigger pin
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(1);
  // Send the trigger signal
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  D_TRACE("TRIG signal sent.");

  // Measure the time (in us) before an echo is received.
  duration = pulseIn(PIN_ECHO, HIGH, WATER_ECHO_TIMEOUT); 
  D_TRACE("ECHO impulse received.");
  
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
    if (distance > WATER_SENSOR_DISTANCE) {
      // Might happen due to imprecision on WATER_SENSOR_DISTANCE or the
      // measured value. Anyway it means is no water in the tank.
      waterLevel = 0;
    } else {
      waterLevel = WATER_SENSOR_DISTANCE - distance;
    }
  }

  // Send the value through the serial port
  D_TRACE("duration (us, one way): " + String(duration));
  D_TRACE("distance (mm): " + String(distance));
  D_TRACE("water level (mm): " + String(waterLevel) + "\n");
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
    pushButtState = digitalRead(PIN_PUMP_BUTTON);

    // The relay is active low and the push button is high when pushed so it is
    // the opposite.
    if (pushButtState == HIGH) {
      D_TRACE("Relay activated.");
    } else {
      D_TRACE("Relay deactivated.");
    }
    digitalWrite(PIN_RELAY, !pushButtState);
  }
}
