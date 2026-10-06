/* 
  bare_metal.cpp
  =====================================
  Implementation of the Automated Plant Watering system using a bare metal approach (no operating system).
  
  For more information about the system, please refer to bare_metal.h.

  Hardware Connections :
    Refer to the main.png image that can be found in the same folder.

  Note:  
    - The pump is only activated for 5s when the soil is considered dry. It may
      not be enough time to humidify the soil correctly. It shouldn't be too
      much impactful because then the soil would be watered again on the next
      polling time until it is considered sufficiently watered. But this 5s
      watering time might be increased in the future if needed.
    
    - The soil moisture sensor value could be refined to be more comprehensive.
      The map() function would be really useful for this.
*/

#if ENV_BARE_METAL == 1

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Arduino.h>
#include "config.h"
#include "macros.h"
#include "../sensors/sensors.h"
#include "../display/display.h"
#include "bare_metal.h"

/*--------------------------------
 * Forward declarations
 *--------------------------------*/
void actionPump(bool toActivate);
void handlePumpButton(void);

/*--------------------------------
 * Global variable declarations
 *--------------------------------*/
// Use init values to consider the system to be fine
uint16_t moistVal = SOIL_DRYNESS_THRESHOLD + 1;
uint16_t lumVal = LIGHT_THRESHOLD_LOW + 1;

// Used to debounce the push buttons
volatile uint32_t lastEdgeTime = 0;

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setupBareMetal()
///
/// Function that will be called only once before loop(). Contains the board,
/// the interrupts, the OLED screen and the serial port initializations.
///
/// parameters: none
///
/// returns: void
///
void setupBareMetal() {
  // Set up the serial port used to display debug traces
  initLogger(9600);
  D_TRACE("Serial port open");

  // Set up the GPIOs
  pinMode(PIN_PUMP_BUTTON, INPUT);
  pinMode(PIN_MOIST_SENSOR, INPUT);
  pinMode(PIN_LUM_SENSOR, INPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  // Turn off the pump at init (active low)
  digitalWrite(PIN_RELAY, HIGH);
  D_TRACE("GPIOs initialized");

  // Initialize the OLED screen
  if (initOledScreen() == -1) {
    D_TRACE("Failed to initialize the OLED screen");
    // Go into an infinite loop to get the user attention
    while(1);
  }

  // Set up the interrupt
  attachInterrupt(digitalPinToInterrupt(PIN_PUMP_BUTTON),
                                        handlePumpButton,
                                        CHANGE);
  D_TRACE("Interrupt initialized");
}

/// loopBareMetal()
///
/// Function that will be looped over indefinitely. Reads the luminosity and
/// the moisture sensor value periodically and activate the pump for 5s if the
/// soil is dry, then deactivate the pump. For security measure, the pump can
/// only be activated if there is enough water in the tank. Displays the
/// information about the system status and the sensor values on an OLED screen.
///
/// parameters: none
///
/// returns: void
///
void loopBareMetal() {
  oled_msg msg;

  moistVal = readMoistureSensor();
  // Update the Soil Moisture value on the screen
  msg.ctx = SOIL_MOISTURE;
  msg.content.data = moistVal;
  buildOledMessage(msg);
  displayOled();
  D_TRACE("Moisture sensor displayed on the screen");

  lumVal = readLuminositySensor();
  // Update the Luminosity value on the screen
  msg.ctx = LUMINOSITY;
  msg.content.data = lumVal;
  buildOledMessage(msg);
  displayOled();
  D_TRACE("Luminosity displayed on the screen");

  if (moistVal > SOIL_DRYNESS_THRESHOLD) {
    // The soil is dry! Turn on the pump for 5s
    D_TRACE("The soil is dry!");

    if (isWaterLevelLow()) {
      // But there is not enough water in the tank, don't turn the pump on
      // Update the status on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = WATER_LOW;
      buildOledMessage(msg);
      displayOled();
      D_TRACE("Status displayed on the screen");
    } else {
      // Update the status on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = SOIL_DRY;
      buildOledMessage(msg);
      displayOled();
      D_TRACE("Status displayed on the screen");

      // Water the plant
      actionPump(true);

      // Update the status on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = PUMP_START;
      buildOledMessage(msg);
      displayOled();
      D_TRACE("Status displayed on the screen");

      D_TRACE("Waiting 5s for the plants to be watered.");
      delay(5000);
      
      // Stop watering the plant
      actionPump(false);

      // Update the status on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = PUMP_DONE;
      buildOledMessage(msg);
      displayOled();
      D_TRACE("Status displayed on the screen");
    }
  } else {
    D_TRACE("The soil is fine.");
    // Update the status on the screen
    msg.ctx = GENERAL_STATUS;
    msg.content.status = SOIL_FINE;
    buildOledMessage(msg);
    displayOled();
    D_TRACE("Status displayed on the screen");
  }
  // The soil moisture will be polled next time based on the value defined in
  // the constant definitions section above. delay() takes the time in ms.
#ifdef DEBUG_MODE
  // While testing poll every 5s
  D_TRACE("Going to sleep...\n");
  delay(5000);
#else
  delay(POLLING_PERIOD_M * 3600);
#endif
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
    D_TRACE("Pump activated!");
  } else {
    digitalWrite(PIN_RELAY, HIGH);
    D_TRACE("Pump deactivated!");
  }
}

/// handlePumpButton()
///
/// ISR that is called when the push button that manually controls the pump is
/// pressed or released. It actually activates or deactivates the relay that
/// controls the water pump, overriding the loop function.
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
  uint32_t currTime = millis();
  bool pushButtState = LOW;

  D_TRACE("Pump Button interrupt.");

  // Need to debounce the push button. As the input may see erratic
  // rising/falling edges after a push on the button, the code will wait a
  // defined delay after the first edge detected.
  if (currTime - lastEdgeTime > DEBOUNCE_DELAY) {
    lastEdgeTime = currTime;
    pushButtState = digitalRead(PIN_PUMP_BUTTON);

    if (pushButtState == HIGH) {
      D_TRACE("Button pushed.");
    } else {
      D_TRACE("Button released.");
    }
    // The relay is active low and the push button is high when pushed so it
    // is the opposite.
    digitalWrite(PIN_RELAY, !pushButtState);
    D_TRACE("Pump activated/deactivated.");
  }
}

#endif // ENV_BARE_METAL == 1
