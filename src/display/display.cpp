/* 
  display.cpp
  =====================================
  Header file of the Display module of the project.
  Read the file header in display.h for more details.
 */

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "macros.h"
#include "types.h"

/*--------------------------------
 * Global variable declarations
 *--------------------------------*/
// OLED screen:
//   (width_pixel, height_pixel, I²C bus to use, no reset pin)
#ifdef I2C_USE_DEFAULT_BUS
static Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
#else
static Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, -1);
#endif // I2C_USE_DEFAULT_BUS

static String persStatusText;
static String persMoistureText;
static String persLuminosityText;

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// displayOled()
///
/// Function to initialize the OLED screen.
/// Read the function declaration in display.h for more details.
///
/// parameters: none
///
/// returns: (int8_t) -1 if failed to initialize the OLED screen, anything else
///   otherwise.
///
int initOledScreen(void) {
#ifndef I2C_USE_DEFAULT_BUS
  // First, needs to open the alt I²C bus
  Wire1.begin(PIN_SCREEN_SDA, PIN_SCREEN_SCL);
#endif // I2C_USE_DEFAULT_BUS

  if(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    return -1;
  }
  D_TRACE("OLED screen initialized");

  // Clean the screen at init
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.display();

  return 1;
}

/// buildOledMessage()
///
/// Function to build the information to be displayed on the OLED screen.
/// displayOled() will have to be called to actually display the information on
/// the screen.
/// Read the function declaration in display.h for more details.
///
/// parameters: 
///   - msg (oled_msg): Contains information to build the message. See 
///     oled_msg type definition.
///
/// returns: void
///
void buildOledMessage(oled_msg msg) {
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

        case WATER_LOW:
          persStatusText = "The water tank is empty. Need a refill!";
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

    case LUMINOSITY:
      if (msg.content.data > LIGHT_THRESHOLD_GREAT) {
        persLuminosityText = "Luminosity: LIGHTMAXXING!";
      } else if (msg.content.data > LIGHT_THRESHOLD_LOW) {
        persLuminosityText = "Luminosity: FINE.";
      } else {
        persLuminosityText = "Luminosity: LOW...";
      } 
      D_TRACE("Luminosity: " + String(msg.content.data));
      break;

    default:
      D_TRACE("[ERROR] Could not find a matching case");
      D_TRACE("Context = " + String(msg.ctx));
      break;
  }
}

/// displayOled()
///
/// Function to display a message on the OLED screen.
/// Read the function declaration in display.h for more details.
///
/// parameters: 
///   - msg (oled_msg): Contains the information to be displayed on the
///     screen. See oled_msg type definition.
///
/// returns: void
///
void displayOled(void) {
  // Now build the whole text to display 
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print(persStatusText);
  oled.setCursor(0, 24);
  oled.print(persMoistureText);
  oled.setCursor(0, 40);
  oled.print(persLuminosityText);
  // Display it on the screen
  oled.display();
}
