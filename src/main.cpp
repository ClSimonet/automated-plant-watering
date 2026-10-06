/* 
  main.cpp
  =====================================
  Implementation of the Automated Plant Watering system using a bare metal approach (no operating system).
  
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
#if ENV_BARE_METAL == 1
#include "bare_metal/bare_metal.h"
#elif ENV_FREERTOS == 1
#include "rtos/rtos.h"
#endif // ENV_BARE_METAL == 1

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that is called only once before loop().
/// Depending on the environment, it either calls the setup function from 
/// bare_metal/bare_metal.cpp or from rtos/rtos.cpp.
///
/// parameters: none
///
/// returns: void
///
void setup() {
#if ENV_BARE_METAL == 1
  setupBareMetal();
#elif ENV_FREERTOS == 1
  setupFreeRtos();
#endif // ENV_BARE_METAL == 1
}

/// loop()
///
/// Function that is looped over indefinitely. 
/// If using the bare_metal environment it calls the loop function from 
/// bare_metal/bare_metal.cpp, otherwise it does nothing because no work is
/// done in the loop function when using the freertos environment as everything
/// is done in the tasks.
///
/// parameters: none
///
/// returns: void
///
void loop() {
#if ENV_BARE_METAL == 1
  loopBareMetal();  
#endif // ENV_BARE_METAL == 1
}
