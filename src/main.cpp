/* 
  main.cpp
  =====================================
  Control the plant watering system. 
  
  The system is composed of 7 units:
    - The Arduino board:
        An Arduino Nano ESP32 board.
    - A water pump:
        Controlled by a relay module activated and deactivated by the Arduino.
    - A soil moisture sensor:
        A capacitive soil moisture sensor that indicates the dryness of the
        plants soil.
    - A luminosity sensor:
        A voltage divider built around a photoresistor that indicates if the
        luminosity received by the plants is good or not.
    - A distance sensor:
        An HC-SR04 ultrasonic sensor that measures the distance between the
        water and the sensor and infers the water level. That way, it prevents
        the pump from being activated if there is no water in the tank.
    - An OLED screen:
        Displays information about the system to the user. Communicates over
        I²C.
    - A push button:
        Overrides the soil moisture sensor and forces the activation of the
        pump.
  
  Hardware Connections :
    Refer to the main.png image that can be found in the assets folder.

  Notes:  
    - The pump is only activated for 5s when the soil is considered dry. It may
      not be enough time to humidify the soil correctly. It shouldn't be too
      much impactful because then the soil would be watered again on the next
      polling time until it is considered sufficiently watered. But this 5s
      watering time might be increased in the future if needed.
    
    - The soil moisture sensor value could be refined to be more comprehensive.
      The map() function would be really useful for this.
*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Arduino.h>
#include "config.h"
#include "macros.h"
#include "sensors/sensors.h"
#include "display/display.h"

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
/// setup()
///
/// Function that will be called only once before loop(). Contains the board,
/// the interrupts, the OLED screenn and the serial port setup.
///
/// parameters: none
///
/// returns: void
///
void setup() {
  // Set up the serial port used to display debug traces
  Serial.begin(9600);
  // Wait for the serial port initialization
  delay(2000);
  D_TRACE("Serial port open");

  // Set up the GPIOs
  pinMode(PIN_PUMP_BUTTON, INPUT);
  pinMode(PIN_MOIST_SENSOR, INPUT);
  pinMode(PIN_LUM_SENSOR, INPUT);
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

/// loop()
///
/// Function that will be looped over indefinitely. Polls the moisture sensor
/// value periodically and activate the pump for 5s if the soil is dry, then
/// deactivate the pump. Also read the luminosity sensor. Display of the
/// information on an OLED screen.
///
/// parameters: none
///
/// returns: void
///
void loop() {
  oled_msg msg;

  moistVal = readMoistureSensor();
  // Update the Soil Moisture value on the screen
  msg.ctx = SOIL_MOISTURE;
  msg.content.data = moistVal;
  displayOled(msg);
  D_TRACE("Moisture sensor displayed on the screen");

  lumVal = readLuminositySensor();
  // Update the Luminosity value on the screen
  msg.ctx = LUMINOSITY;
  msg.content.data = lumVal;
  displayOled(msg);
  D_TRACE("Luminosity displayed on the screen");

  if (moistVal > SOIL_DRYNESS_THRESHOLD) {
    // The soil is dry! Turn on the pump for 5s
    D_TRACE("The soil is dry!");

    if (isWaterLevelLow()) {
      // But there is not enough water in the tank, don't turn the pump on
      // Update the status on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = WATER_LOW;
      displayOled(msg);
      D_TRACE("Status displayed on the screen");
    } else {
      // Update the status on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = SOIL_DRY;
      displayOled(msg);
      D_TRACE("Status displayed on the screen");

      // Water the plant
      actionPump(true);

      // Update the status on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = PUMP_START;
      displayOled(msg);
      D_TRACE("Status displayed on the screen");

      D_TRACE("Waiting 5s for the plants to be watered.");
      delay(5000);
      
      // Stop watering the plant
      actionPump(false);

      // Update the status on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = PUMP_DONE;
      displayOled(msg);
      D_TRACE("Status displayed on the screen");
    }
  } else {
    D_TRACE("The soil is fine.");
    // Update the status on the screen
    msg.ctx = GENERAL_STATUS;
    msg.content.status = SOIL_FINE;
    displayOled(msg);
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
