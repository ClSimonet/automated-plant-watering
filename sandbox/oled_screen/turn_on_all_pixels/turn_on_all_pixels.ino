/* 
  turn_on_all_pixels.ino
  =====================================
  Code used to test the OLED screen. It turns on all the pixels on the screen, then after a few seconds it turns them all off.
  If USE_DRAW_PIXEL is defined, the code will use oled.drawPixel(). Otherwise it will use oled.fillRect().

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

//#define USE_DRAW_PIXEL        // Used to switch between using oled.drawPixel() or oled.fillRect()


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
  }
  else {
    Serial.println("Successfully initialized the OLED screen.");
  }
  oled.clearDisplay();  
}

/// loop()
///
/// Function that will be looped over indefinitely.
/// Turns on and off all the pixel on the OLED screen. 
/// If USE_DRAW_PIXEL is defined, the code will use oled.drawPixel(). 
/// Otherwise it will use oled.fillRect().
///
/// parameters:
///     none
///
/// returns:
///     void
///
void loop() {
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
  delay(2000);
  // Do it again but turn off the screen
  oled.clearDisplay();
  for(uint8_t iterRow = 0 ; iterRow < SCREEN_HEIGHT ; iterRow++) {
    for(uint8_t iterCol = 0 ; iterCol < SCREEN_WIDTH ; iterCol++) {
      oled.drawPixel(iterCol, iterRow, BLACK);
    }
  }
  oled.display();
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