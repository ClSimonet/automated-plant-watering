/* 
  display.h
  =====================================
  Header file of the Display module of the project.
  The project uses only one display based on an SSD1306 device. The OLED screen
  address is 0x3C.

  This module heavily relies on the Adafruit_SSD1306 library.  
 */

#ifndef DISPLAY_H
#define DISPLAY_H

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include "types.h"

/// displayOled()
///
/// Function to initialize the OLED screen.
/// First, if the user required to use a different bus than the default one 
/// (i.e. if I2C_USE_DEFAULT_BUS is defined), then the function will open the
/// new I²C bus using the PIN_SCREEN_SDA and PIN_SCREEN_SCL pins defined in 
/// inc/config.h.
/// After that it will turn on the OLED screen and display a blank screen.
///
/// parameters: none
///
/// returns: (int8_t) -1 if failed to initialize the OLED screen, anything else
///   otherwise.
///
int initOledScreen(void);

/// displayOled()
///
/// Function to display a message on the OLED screen.
/// Every time this function is called, it will regenerate the whole screen to
/// display (not only the information contained in `msg`). That is because
/// clearDisplay() is needed to get rid of the old information but it
/// completely empties the OLED screen buffer thus all the other information
/// would be lost. To avoid that, every information received is saved into a
/// persistent String that will be used to regenerate the whole screen.
///
/// Note: Another solution to the clearDisplay() problem might be to use
///   print("") on the related line, then to use it again with the new
///   information. Even though the current solution is less efficient, it was
///   deemed more comprehensible. This might be a point of optimization.
///
/// parameters: 
///   - msg (oled_msg): Contains the information to be displayed on the
///     screen. See oled_msg type definition.
///
/// returns: void
///
void displayOled(oled_msg msg);

#endif // DISPLAY_H
