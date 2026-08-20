/* 
  print_hello_world.ino
  =====================================
  Code used to test the OLED screen. It just tests the oled.print() function
  by printing different dummy basic messages on the screen.
 
  Hardware Connections :
  Refer to the oled_screen.png image that can be found in the parent folder folder.

  Notes:  
  - The OLED screen address is 0x3C.
*/
/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Adafruit_SSD1306.h>


/*--------------------------------
 * Constant definitions
 *--------------------------------*/
#define SCREEN_WIDTH      128         // OLED screen width, in pixels
#define SCREEN_HEIGHT     64          // OLED screen height, in pixels
#define SCREEN_I2C_ADDR   0x3C        // OLED screen address used to communicate over I²C


/*--------------------------------
 * Global variable definitions
 *--------------------------------*/
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


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
void setup() {
  // Set up the serial port used to display debug traces
  Serial.begin(9600);
  // Wait for the serial port initialization before using it
  delay(2000);

  // Initialize the OLED screen
  if(!oled.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
    Serial.println("Failed to Initialize the OLED screen. Stop here (loop indefinitely).");
    while(1);
  } else {
    Serial.println("Successfully initialized the OLED screen.");
  }
  oled.clearDisplay();  
}

/// loop()
///
/// Function that will be looped over indefinitely.
/// Display messages on the OLED screen using the oled.print().
///
/// parameters:
///     none
///
/// returns:
///     void
///
void loop() {
  // Clear the screen buffer before using it
  oled.clearDisplay();
  oled.setTextColor(WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print("It's a shy Hello World...");
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
  oled.print("It's a surprising HeLlO wOrLd");
  oled.display();
  delay(2000);  
}