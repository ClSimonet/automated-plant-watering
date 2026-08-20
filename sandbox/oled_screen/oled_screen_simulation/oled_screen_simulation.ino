/* 
  oled_screen_simulation.ino
  =====================================
  Code used to test the OLED screen 
  It simulate a typical scenario in the context of the Automated Plant
  Watering System.

  The goal is to print a message like:
  WATERING...
  Luminosity: X%
  Soil Moisture: X%
  
  Hardware Connections :
  Refer to the oled_screen.png image that can be found in the parent folder folder.

  Notes:  
  - The DEBUG_MODE constant allows the user to see debug traces. To do so the
    user must uncomment the "//#define DEBUG_MODE" line below in the
    "Constant definitions" section and open the serial port at 9600 baud in
    the Arduino IDE (Tools > Open Serial Monitor).

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
// Output pins
#define PIN_SCREEN_SDA      A4
#define PIN_SCREEN_SCK      A5

// OLED Screen constants
#define SCREEN_WIDTH        128         // OLED screen width, in pixels
#define SCREEN_HEIGHT       64          // OLED screen height, in pixels
#define SCREEN_I2C_ADDR     0x3C        // OLED screen address used to communicate over I²C

// If defined, then the D_TRACE() macro will print useful information on the
// serial port for debugging purpose
//#define DEBUG_MODE


/*--------------------------------
 * Macro definitions
 *--------------------------------*/
#ifdef DEBUG_MODE
#define D_TRACE(s) do {         \
  Serial.print(F("[DEBUG] "));  \
  Serial.print(__FILE__);       \
  Serial.print(F("@"));         \
  Serial.print(__LINE__);       \
  Serial.print(F(": "));        \
  Serial.println(s);            \
} while(0)
#else
#define D_TRACE(s)
#endif


/*--------------------------------
 * Type definitions
 *--------------------------------*/
// Used to indicate in which context the message must be displayed. It can be:
//   - GENERAL_STATUS: The message contains useful information for the user
//   - SOIL_MOISTURE: The message contains the soil moisture sensor value
//   - LUMINOSITY: The message contains the soil moisture sensor value
typedef enum {
  GENERAL_STATUS,
  SOIL_MOISTURE,
  LUMINOSITY
} oled_msg_ctx;

// Type passed as parameter to displayOled() in order to print a message
// correctly on the screen.
typedef struct {
  // Indicate the purpose of the message
  oled_msg_ctx ctx;

  // Carry the information to be displayed on screen
  union {
    // Only valid if ctx = GENERAL_STATUS.
    // The upper layer must provide an allocated string and it must stay valid
    // until displayOled() returns 
    char *text;

    // Only valid if ctx = SOIL_MOISTURE or LUMINOSITY
    uint8_t data;
  } content;
} oled_msg;


/*--------------------------------
 * Global variable definitions
 *--------------------------------*/
// OLED screen declaration: 
//   (width_pixel, height_pixel, used for I²C communication, no reset pin)
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

uint8_t testMoistVal = 10;
uint8_t testLumVal = 3;


/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that will be called only once before loop(). 
/// Contains the board and OLED screen initialization.
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

  // Set up the GPIOs
  pinMode(PIN_SCREEN_SDA, OUTPUT);
  pinMode(PIN_SCREEN_SCK, OUTPUT);

  // Initialize the OLED screen
  D_TRACE("Initializing the OLED screen");
  delay(2000);
  if(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    D_TRACE("Failed to Initialize the OLED screen");
    while(1); // Stop there
  }
  D_TRACE("OLED screen initialized");
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
}

/// loop()
///
/// Function that will be looped over indefinitely.
/// Simulate a watering station that sends the information to 
/// displayOled().
///
/// parameters:
///     none
///
/// returns:
///     void
///
void loop(void) {
  oled_msg msg;

  // Update fake measurements values
  testMoistVal += 10;
  // Rollover if greater than 80
  if (testLumVal < 80) {
    testLumVal += 15;
  } else {
    testLumVal = 3;
  }

  // Update the Soil Moisture value on the screen
  msg.ctx = SOIL_MOISTURE;
  msg.content.data = testMoistVal;
  displayOled(msg);
  delay(2000);

  // Update the Luminosity value on the screen
  msg.ctx = LUMINOSITY;
  msg.content.data = testLumVal;
  displayOled(msg);
  delay(2000);

  if (testMoistVal > 50) {
    // Update the Status message on the screen
    msg.ctx = GENERAL_STATUS;
    msg.content.text = "Watering...";
    displayOled(msg);
    delay(3000);

    // Update the Status message on the screen
    msg.ctx = GENERAL_STATUS;
    msg.content.text = "The water tank needs to be refilled!";
    displayOled(msg);
    delay(3000);

    // Update the Status message value on the screen
    msg.ctx = GENERAL_STATUS;
    msg.content.text = "Everything's fine.";
    displayOled(msg);

    // Reinit the moisture value to repeat the test
    testMoistVal = 10;
  }
}

/// displayOled()
///
/// Function to display a message on the OLED screen.
/// Display a message on the appropriate line of the OLED screen depending if
/// it pertains to the luminosity, to the soil moisture, or to the general
/// status of the system.
///
/// parameters: 
///   - msg (oled_msg): Contains the information to be displayed on the
///     screen. See oled_msg type definition.
///
/// returns: 
///     none
///
void displayOled(oled_msg msg) {
  // Use static String to save the last message for each context. That way, the
  // OLED buffer can be filled with all the information at once and they can
  // all be printed. Otherwise oled.clearDisplay() would throw away these
  // information and only one line at a time would be printed on the screen.
  static String persStatusText;
  static String persMoistureText;
  static String persLuminosityText;

  D_TRACE("displayOled() called with context " + String(msg.ctx) + " to print: ");

  // Update the persistent messages that will be displayed
  switch(msg.ctx) {
    case GENERAL_STATUS:
      persStatusText = msg.content.text;
      D_TRACE(String(msg.content.text));
      break;

    case SOIL_MOISTURE:
      persMoistureText = "Soil Moisture: " + String(msg.content.data) + "%";
      D_TRACE("Soil Moisture: " + String(msg.content.data) + "%");
      break;

    case LUMINOSITY:
      persLuminosityText = "Luminosity: " + String(msg.content.data) + "%";
      D_TRACE("Luminosity: " + String(msg.content.data) + "%");
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
  oled.setCursor(0, 32);
  oled.print(persLuminosityText);
  // Display it on the screen
  oled.display();
}
