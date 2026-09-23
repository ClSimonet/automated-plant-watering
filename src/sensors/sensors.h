/* 
  sensors.h
  =====================================
  Header file of the Sensors module of the project.
  The project uses 3 sensors:
    - a soil moisture sensor
    - a luminosity sensor
    - a distance sensor to measure the water level

  The soil moisture sensor is a capacitive soil moisture sensor and is placed in the plants soil. It indicates if the plants need water or not. 
  It is powered in 3.3V. The value is read on an Arduino analog pin.
  As references, the soil moisture sensor returned the following values:
    § dipped into water:    1120
    § into a wet soil:      1200
    § into a fine soil:     1900
    § into a very dry soil: 3280
    § dry out in the air:   3300

  The soil moisture sensor is a photoresistor with a voltage divider built around it, and is placed next to the plants. A 10k Ohm resistor is used for the voltage divider. The montage indicates how much luminosity is received by the plants.
  It is powered in 3.3V. The value is read on an Arduino analog pin.
  As references, the luminosity sensor returned the following values:
    § outside directly exposed to the sun:  4095
    § outside in the shades:                3900
    § outside at night:                     0
    § inside in a dark room:                0
    § in a room with natural light:         3200
    § in a room with artificial light:      3000

  The distance sensor is an HC-SR04 module which is an ultrasonic sensor and is placed above the water tank. It sends a trigger impulse signal, and measures the time before an echo is received. From there it can infer the distance between the distance between the sensor and the water thus the water level in the tank.
  It is powered in 3.3V. The trigger impulse is sent on an Arduino digital pin and the echo is received on an Arduino digital pin.
*/

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
/// returns: (uint16_t) raw soil moisture value read from the sensor
///
uint16_t readMoistureSensor(void);

/// readLuminositySensor()
///
/// Function to read the value returned by the luminosity sensor.
///
/// Note: The function does very little and might seem useless for now, but
/// the design of using a function to read the sensor will be more valuable
/// when the project gets more complex.
///
/// parameters: none
///
/// returns: (uint16_t) raw luminosity value read from the sensor
///
uint16_t readLuminositySensor(void);

/// isWaterLevelLow()
///
/// Function to determine if the water level in the tank is low or not.
/// It uses the SR-HC
///
/// parameters: none
///
/// returns: (bool) true if the water level is lower than 1 cm, false otherwise
///
bool isWaterLevelLow(void);