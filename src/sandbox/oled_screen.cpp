/* 
  oled_screen.cpp
  =====================================
  Code used to test and play with the oled screen. 
  This code contains 4 small programs to demonstrate the use of the OLED screen:
    - i2c_scanner() which finds all the I²C devices connected to the board. It
    tries to open an I²C connection using all the possible addresses and send
    the address of the successful cases on the serial port.
    - switch_all_pixels() which turns all the pixels of the screen on for 2
    seconds, then turn them off. It can either achieve it using oled.drawPixel()
    or oled.fillRect().
    - print_hello_worlds() which will print several "Hello World!" messages
    alternating between different sizes and places on the screen.
    - watering_plant_simulation() which simulates a typical scenario of the
    final automated watering plant watering project.

  The constants in the Constant definitions section are used to select which
  program to run. Read their description for more information.

  There is also data sent over the serial port at 9600 baud/s.

  Hardware Connections :
    Refer to the oled_screen.png image that can be found in the assets folder.
  
  Notes:  
  
*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "macros.h"
#include "types.h"

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
// If I2C_SCANNER is defined, the i2c_scanner() function will run.
//#define I2C_SCANNER

// If SWITCH_ALL_PIXEL is defined, the switch_all_pixels() function will run.
// Needs I2C_SCANNER to not be defined.
//#define SWITCH_ALL_PIXEL
// If SWITCH_ALL_PIXEL is defined, the switch_all_pixels() function will use
// oled.drawPixel(), otherwise it will use oled.fillRect().
// Needs SWITCH_ALL_PIXEL to be defined.
//#define USE_DRAW_PIXEL

// If PRINT_HELLO_WORLDS is defined, the print_hello_worlds() function will run.
// Needs I2C_SCANNER and SWITCH_ALL_PIXEL to not be defined.
//#define PRINT_HELLO_WORLDS

// If WATERING_PLANT_SIMULATION is defined, the watering_plant_simulation()
// function will run.
// Needs I2C_SCANNER, SWITCH_ALL_PIXEL and PRINT_HELLO_WORLDS to not be defined.
#define WATERING_PLANT_SIMULATION

/*--------------------------------
 * Variable declarations
 *--------------------------------*/
// OLED screen:
//   (width_pixel, height_pixel, I²C bus to use, no reset pin)
static Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/*--------------------------------
 * Forward declaration
 *--------------------------------*/
#ifdef I2C_SCANNER
void i2c_scanner(void);
#endif // I2C_SCANNER

#ifdef SWITCH_ALL_PIXEL
void switch_all_pixels(void);
#endif // SWITCH_ALL_PIXEL

#ifdef PRINT_HELLO_WORLDS
void print_hello_worlds(void);
#endif // PRINT_HELLO_WORLDS

#ifdef WATERING_PLANT_SIMULATION
void watering_plant_simulation(void);
void watering_plant_display_oled(oled_msg msg);
#endif // WATERING_PLANT_SIMULATION

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that will be called only once before loop(). 
/// Contains the board and I²C initialization.
///
/// parameters:
///     none
///
/// returns:
///     void
///
void setup() {
  // Set up the serial port used to display the value on the screen
  Serial.begin(9600);
  // Wait for the serial port initialization
  delay(2000);

  // Initialize the I²C bus
  Wire.begin();

  // Only I²C Scanner will not initialize the screen
#ifndef I2C_SCANNER
  // Initialize the OLED screen
  if(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    D_TRACE("Failed to Initialize the OLED screen. Stop here (loop indefinitely).");
    while(1);
  }
  else {
    D_TRACE("Successfully initialized the OLED screen.");
  }
  oled.clearDisplay();  
#endif // I2C_SCANNER

  D_TRACE("Setup done.");

#ifdef I2C_SCANNER
  D_TRACE("Running I²C Scanner...");
#else
#ifdef SWITCH_ALL_PIXEL
  D_TRACE("Running Switch All Pixels...");
#else
#ifdef PRINT_HELLO_WORLDS
  D_TRACE("Running Print Hello Worlds...");
#else
#ifdef WATERING_PLANT_SIMULATION
  D_TRACE("Running Watering Plant Simulation...");
#endif // WATERING_PLANT_SIMULATION
#endif // PRINT_HELLO_WORLDS
#endif // SWITCH_ALL_PIXEL
#endif // I2C_SCANNER
}

/// loop()
///
/// Function that will be looped over indefinitely.
/// Try to communicate over every I²C addresses possible and print on the
/// serial port all the address of the devices that respond.
///
/// parameters:
///     none
///
/// returns:
///     void
///
void loop() {
#ifdef I2C_SCANNER
  i2c_scanner();
#else
#ifdef SWITCH_ALL_PIXEL
  switch_all_pixels();
#else
#ifdef PRINT_HELLO_WORLDS
  print_hello_worlds();
#else
#ifdef WATERING_PLANT_SIMULATION
  watering_plant_simulation();
#endif // WATERING_PLANT_SIMULATION
#endif // PRINT_HELLO_WORLDS
#endif // SWITCH_ALL_PIXEL
#endif // I2C_SCANNER
}

#ifdef I2C_SCANNER
/// i2c_scanner()
///
/// Function to find the address of the OLED screen.
/// It tries to open a connection on every possible address, and return the
/// address of the successful cases.
///
/// parameters: none
///
/// returns: void
///
void i2c_scanner() {
  byte error, address;
  int nDevices = 0;

  Serial.println("\nScanning...");

  for (address = 1; address < 127; address++) {
    // Test this address
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      // The device responded. There is somebody at this address
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.println("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }
  }

  if (nDevices == 0) {
    D_TRACE("No I2C devices found.");
  }

  delay(5000);
}
#endif // I2C_SCANNER

#ifdef SWITCH_ALL_PIXEL
/// switch_all_pixels()
///
/// Function to switch all the pixels on the OLED screen.
/// It turns on all the pixels on the screen, then after a few seconds it turns
/// them all off.
/// If USE_DRAW_PIXEL is defined, the code will use oled.drawPixel(). Otherwise
/// it will use oled.fillRect().
///
/// parameters: none
///
/// returns: void
///
void switch_all_pixels() {
#ifdef USE_DRAW_PIXEL
  // Clear the screen buffer before using it
  oled.clearDisplay();
  for(uint8_t iterRow = 0 ; iterRow < SCREEN_HEIGHT ; iterRow++) {
    for(uint8_t iterCol = 0 ; iterCol < SCREEN_WIDTH ; iterCol++) {
      // Load the buffer by turning each pixel on, one by one.
      oled.drawPixel(iterCol, iterRow, WHITE);
    }
  }
  // Print the buffer on the screen
  oled.display();
  D_TRACE("Turned ON");
  delay(2000);
  // Do it again but turn off the screen
  oled.clearDisplay();
  for(uint8_t iterRow = 0 ; iterRow < SCREEN_HEIGHT ; iterRow++) {
    for(uint8_t iterCol = 0 ; iterCol < SCREEN_WIDTH ; iterCol++) {
      oled.drawPixel(iterCol, iterRow, BLACK);
    }
  }
  oled.display();
  D_TRACE("Turned OFF");
#else
  // Clear the screen buffer before using it
  oled.clearDisplay();
  // Load the buffer by drawing a rectangle with the size of the screen
  oled.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
  // Print the buffer on the screen
  oled.display();
  delay(2000);
  // Do it again but turn off the screen
  oled.clearDisplay();
  oled.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  oled.display();
#endif
  delay(2000);
}
#endif // SWITCH_ALL_PIXEL

#ifdef PRINT_HELLO_WORLDS
/// print_hello_worlds()
///
/// Function to display a few "Hello World!"" variant messages on the OLED
/// screen.
/// It uses the oled.print() function to print different dummy basic messages
/// on the screen changing the size and/or the place on the screen.
///
/// parameters: none
///
/// returns: void
///
void print_hello_worlds() {
  // Clear the screen buffer before using it
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("It's a shy hello world...");
  oled.display();
  delay(2000);

  oled.clearDisplay();
  oled.setTextSize(2);
  oled.print("It's a confident HELLO WORLD!");
  oled.display();
  delay(2000);
  
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(0, 48);
  oled.print("It's a weird HeLlO wOrLd");
  oled.display();
  delay(2000);  
}
#endif // PRINT_HELLO_WORLDS

#ifdef WATERING_PLANT_SIMULATION
/// watering_plant_simulation()
///
/// Function to use the OLED screen in the context of the Automated Watering
/// Plant system.
/// It simulates a typical scenario in the context of the Automated Plant
/// Watering system with sensor values. Then calls the watering_plant_display_oled() to print
/// the system information on the OLED screen. 
/// Refer to the header of watering_plant_display_oled() for more information about this
/// function.
///
/// parameters: none
///
/// returns: void
///
void watering_plant_simulation() {
  oled_msg msg;
  static uint8_t testMoistVal = 10;
  static uint8_t testLumVal = 3;

  // Update fake measurements values
  testMoistVal += 10;
  if (testLumVal < 80) {
    testLumVal += 15;
  } else {
    // Rollover if greater than 80
    testLumVal = 3;
  }

  // Update the Soil Moisture value on the screen
  msg.ctx = SOIL_MOISTURE;
  msg.content.data = testMoistVal;
  watering_plant_display_oled(msg);

  // Update the Luminosity value on the screen
  msg.ctx = LUMINOSITY;
  msg.content.data = testLumVal;
  watering_plant_display_oled(msg);
  delay(2000);

  if (testMoistVal > 50) {
    // Simulate the soil being dry
    msg.ctx = GENERAL_STATUS;
    msg.content.status = SOIL_DRY;
    watering_plant_display_oled(msg);
    delay(2000);

    // Simulate not enough water in the tank
    msg.ctx = GENERAL_STATUS;
    msg.content.status = WATER_LOW;
    watering_plant_display_oled(msg);
    delay(2000);

    // Simulate the pump being turned on for 5 seconds
    msg.ctx = GENERAL_STATUS;
    msg.content.status = PUMP_START;
    watering_plant_display_oled(msg);
    delay(5000);

    // Simulate the pump being turned off
    msg.ctx = GENERAL_STATUS;
    msg.content.status = PUMP_DONE;
    watering_plant_display_oled(msg);

    // Reinit the moisture value to repeat the test
    testMoistVal = 10;
  } else {
    // Simulate the soil being fine
    msg.ctx = GENERAL_STATUS;
    msg.content.status = SOIL_FINE;
    watering_plant_display_oled(msg);
  }

  delay(2000);
}

/// watering_plant_display_oled()
///
/// Function to display a message on the OLED screen.
/// Display a message on the appropriate line of the OLED screen depending if
/// it pertains to the luminosity, to the soil moisture, or to the general
/// status of the system.
/// Use static String to save the last message for each context. That way, the
/// OLED buffer can be filled with all the information at once and they can
/// all be printed. Otherwise oled.clearDisplay() would throw away these
/// information and only one line at a time would be printed on the screen.
///
/// parameters: 
///   - msg (oled_msg): Contains the information to be displayed on the
///     screen. See oled_msg type definition.
///
/// returns: 
///     none
///
void watering_plant_display_oled(oled_msg msg) {
  // Use static String to save the last message for each context. That way, the
  // OLED buffer can be filled with all the information at once and they can
  // all be printed. Otherwise oled.clearDisplay() would throw away these
  // information and only one line at a time would be printed on the screen.
  static String persStatusText;
  static String persMoistureText;
  static String persLuminosityText;

  D_TRACE("watering_plant_display_oled() called with context " + String(msg.ctx) + " to print: ");

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
      persMoistureText = "Soil Moisture: " + String(msg.content.data) + "%";
      D_TRACE(persMoistureText);
      break;

    case LUMINOSITY:
      persLuminosityText = "Luminosity: " + String(msg.content.data) + "%";
      D_TRACE(persLuminosityText);
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
#endif // WATERING_PLANT_SIMULATION
