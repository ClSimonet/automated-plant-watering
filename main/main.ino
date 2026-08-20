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

  - The DEBUG_MODE constant enables debug traces. To do so, the 
    `//#define DEBUG_MODE` line in the "Constant definitions" section must be
    uncommented and the serial port be opened at 9600 baud in the Arduino IDE
    (Tools > Open Serial Monitor).

  - The I2C_USE_DEFAULT_BUS constant allows the user to use another I²C bus.
    By default, the A4 and A5 pins are used to communicate over I²C with the
    OLED screen. But it is possible to use a different I²C bus and assign it
    to different pins (to free the analog pins for instance). To do so, the 
    `//#define I2C_USE_DEFAULT_BUS` line in the "Constant definitions" section
    must be uncommented and the `PIN_SCREEN_SDA` and `PIN_SCREEN_SCL`
    constants must be set to the desired pins.

  - The OLED screen address is 0x3C.
*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>


/*--------------------------------
 * Constant definitions
 *--------------------------------*/
#define DEBUG_MODE
#define I2C_USE_DEFAULT_BUS

//--- Input pins ---//
#define PIN_PUMP_BUTTON     D4
#define PIN_MOIST_SENSOR    A0

//--- Output pins ---//
// The relay directly commands the water pump
#define PIN_RELAY           D12

//--- I²C pins ---//
// If I2C_USE_DEFAULT_BUS is defined, use different pins to communicate with
// the OLED screen.
#ifndef I2C_USE_DEFAULT_BUS
#define PIN_SCREEN_SDA      D2
#define PIN_SCREEN_SCL      D3
#endif // I2C_USE_DEFAULT_BUS

//--- Constants ---//
// Represents the dryness of the soil. A value greater than
// BASE_DRY_THRESHOLD indicates the soil is dry. After calibrating the
// sensor, the threshold is considered to be 1900.
#define BASE_DRY_THRESHOLD  1900
// Period (in minutes) at which the moisture sensor will be read
#define POLLING_PERIOD_M    30
// Time in ms to wait to avoid bounces on the push button
#define DEBOUNCE_DELAY      50
// OLED screen width, in pixels
#define SCREEN_WIDTH        128         
// OLED screen height, in pixels
#define SCREEN_HEIGHT       64          
// OLED screen address used to communicate over I²C
#define SCREEN_I2C_ADDR     0x3C        


/*--------------------------------
 * Type definitions
 *--------------------------------*/
// Used to indicate in which context the message must be displayed. It can be:
//   - GENERAL_STATUS: The message contains useful information for the user
//   - SOIL_MOISTURE: The message contains the soil moisture sensor value
typedef enum {
  GENERAL_STATUS,
  SOIL_MOISTURE,
} oled_msg_ctx;

// Used to describe the current status of the system. It can be:
//   - INIT: The system is currently initializing
//   - SOIL_FINE: The soil moisture has been measured and the soil is fine
//   - SOIL_DRY: The soil moisture has been measured and the soil is dry
//   - PUMP_START: The system is currently watering the plants
//   - PUMP_DONE: The system is done watering the plants
typedef enum {
  INIT,
  SOIL_FINE,
  SOIL_DRY,
  PUMP_START,
  PUMP_DONE,
} gen_status;

// Type passed as parameter to displayOled() in order to print a message
// correctly on the screen.
typedef struct {
  // Indicate the purpose of the message
  oled_msg_ctx ctx;

  // Carry the information to be displayed on screen
  union {
    // Only valid if ctx = GENERAL_STATUS.
    gen_status status;

    // Only valid if ctx = SOIL_MOISTURE
    uint32_t data;
  } content;
} oled_msg;


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
 * Global variable declarations
 *--------------------------------*/
// OLED screen:
//   (width_pixel, height_pixel, I²C bus to use, no reset pin)
#ifdef I2C_USE_DEFAULT_BUS
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#else
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, -1);
#endif // I2C_USE_DEFAULT_BUS

// Value to consider the soil to be ok at init
uint16_t moistVal = BASE_DRY_THRESHOLD;

// Used to debounce the push buttons
volatile uint32_t lastEdgeTime = 0;


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
  // Set up the serial port used to display debug traces
  Serial.begin(9600);
  // Wait for the serial port initialization
  delay(2000);
  D_TRACE("Serial port open");

  // Set up the GPIOs
  pinMode(PIN_PUMP_BUTTON, INPUT);
  pinMode(PIN_MOIST_SENSOR, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  D_TRACE("GPIOs initialized");

  // Initialize the OLED screen
#ifndef I2C_USE_DEFAULT_BUS
  Wire1.begin(PIN_SCREEN_SDA, PIN_SCREEN_SCL);
#endif // I2C_USE_DEFAULT_BUS
  if(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    D_TRACE("Failed to Initialize the OLED screen");
    while(1); // Stop there
  }
  D_TRACE("OLED screen initialized");
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);

  // Set up the interrupt
  attachInterrupt(digitalPinToInterrupt(PIN_PUMP_BUTTON), handlePumpButton, CHANGE);
  D_TRACE("Interrupt initialized");
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
  oled_msg msg;

  readMoistureSensor();

  // Update the Soil Moisture value on the screen
  msg.ctx = SOIL_MOISTURE;
  msg.content.data = moistVal;
  displayOled(msg);
  D_TRACE("Moisture sensor displayed on the screen");

  if (moistVal > BASE_DRY_THRESHOLD) {
    // The soil is dry! Turn on the pump for 5s
    D_TRACE("The soil is dry!");

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

    delay(5000);
    
    // Stop watering the plant
    actionPump(false);

    // Update the status on the screen
    msg.ctx = GENERAL_STATUS;
    msg.content.status = PUMP_DONE;
    displayOled(msg);
    D_TRACE("Status displayed on the screen");
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

/// readMoistureSensor()
///
/// Function to read the value returned by the soil moisture sensor.
///
/// Note: The function does very little and might seem useless for now, but
/// the design of using a function to read the sensor will be more valuable
/// when the project gets more complex.
///
/// parameters: none
///
/// returns: none
///
void readMoistureSensor() {
  moistVal = analogRead(PIN_MOIST_SENSOR);
  D_TRACE("Measured moisture sensor value: " + String(moistVal));
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
  uint32_t currTime = millis();
  // Just a default value
  bool pushButtState = LOW;

  D_TRACE("Button pushed!");

  // Need to debounce the push button. As the input may see erratic
  // rising/falling edges after a push on the button, the code will wait a
  // defined delay after the first edge detected.
  if (currTime - lastEdgeTime > DEBOUNCE_DELAY) {
    lastEdgeTime = currTime;
    pushButtState = digitalRead(PIN_PUMP_BUTTON);

    // The relay is active low and the push button is high when pushed so it
    // is the opposite.
    digitalWrite(PIN_RELAY, !pushButtState);
    D_TRACE("Forced pump activation...");
  }
}

/// displayOled()
///
/// Function to display a message on the OLED screen.
/// Display a message on the appropriate line of the OLED screen depending if
/// it pertains to the soil moisture, or to the general status of the system.
///
/// parameters: 
///   - msg (oled_msg): Contains the information to be displayed on the
///     screen. See oled_msg type definition.
///
/// returns: 
///     none
///
void displayOled(oled_msg msg) {
  static String persStatusText;
  static String persMoistureText;

  D_TRACE("displayOled() called with context " + String(msg.ctx) + " to print: ");

  // Update the persistent messages that will be displayed
  switch(msg.ctx) {
    case GENERAL_STATUS:
      switch(msg.content.status) {
        case INIT:
          persStatusText = "Initialization...";
          break;

        case SOIL_FINE:
          persStatusText = "Everything is fine!";
          break;

        case SOIL_DRY:
          persStatusText = "The soil is dry!";
          break;

        case PUMP_START:
          persStatusText = "Watering the plants...";
          break;

        case PUMP_DONE:
          persStatusText = "Done watering the plants.";
          break;
      }
      D_TRACE(persStatusText);
      break;

    case SOIL_MOISTURE:
      persMoistureText = "Soil Moisture: " + String(msg.content.data);
      D_TRACE("Soil Moisture: " + String(msg.content.data));
      break;

    default:
      D_TRACE("[ERROR] Could not find a matching case");
      D_TRACE("Context = " + String(msg.ctx));
      break;
  }

  // Now build the whole text to display 
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print(persStatusText);
  oled.setCursor(0, 16);
  oled.print(persMoistureText);
  // Display it on the screen
  oled.display();
}
