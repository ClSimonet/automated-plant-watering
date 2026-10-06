/* 
  bare_metal.h
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
    Refer to the main.png image that can be found in the same folder.

  Note:  
    - The pump is only activated for 5s when the soil is considered dry. It may
      not be enough time to humidify the soil correctly. It shouldn't be too
      much impactful because then the soil would be watered again on the next
      polling time until it is considered sufficiently watered. But this 5s
      watering time might be increased in the future if needed.
    
    - The soil moisture sensor value could be refined to be more comprehensive.
      The map() function would be really useful for this.
*/
#ifndef BARE_METAL_H
#define BARE_METAL_H

/*--------------------------------
 * Function declarations
 *--------------------------------*/
/// setupBareMetal()
///
/// Function that will be called only once before loop(). Contains the board,
/// the interrupts, the OLED screen and the serial port setup.
///
/// parameters: none
///
/// returns: void
///
void setupBareMetal(void);

/// loopBareMetal()
///
/// Function that will be looped over indefinitely. Reads the luminosity and
/// the moisture sensor value periodically and activate the pump for 5s if the
/// soil is dry, then deactivate the pump. For security measure, the pump can
/// only be activated if there is enough water in the tank. Displays the
/// information about the system status and the sensor values on an OLED screen.
///
/// parameters: none
///
/// returns: void
///
void loopBareMetal(void);

#endif // BARE_METAL_H
