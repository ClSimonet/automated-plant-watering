/* 
  oled_screen_simulation.ino
  =====================================
  Code used to find the address of the OLED screen.
  It will try to open a connection on every possible address, and return the
  address of the successful cases.
    
  Hardware Connections :
  Refer to the oled_screen.png image that can be found in the parent folder folder.

  Notes:  
  - The DEBUG_MODE constant allows the user to see debug traces. To do so the
    user must uncomment the "//#define DEBUG_MODE" line below in the
    "Constant definitions" section and open the serial port at 9600 baud in
    the Arduino IDE (Tools > Open Serial Monitor).
*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <Wire.h>


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
  Serial.begin(9600);
  Wire.begin();
  Serial.println("Scanning I2C devices...");
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
  byte error, address;
  int nDevices = 0;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {
    // Test this address
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      // The device responded. There is somebody at this address
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found.");
  }

  delay(5000);
}