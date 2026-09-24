/* 
  config.h
  =====================================
  Header file containing the constant definitions, the Arduino board pin layout.
 */

#ifndef CONFIG_H
#define CONFIG_H

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
//--- Configuration constants ---//
// If defined, enables debug traces. To do so, the serial port must be opened 
// at 9600 baud/s.
#define DEBUG_MODE
// If not defined, allows the user to use another I²C bus. By default, the A4
// and A5 pins are used to communicate over I²C with the OLED screen. But it is
// possible to use a different I²C bus and assign it to different pins (to free
// the analog pins for instance). The new I²C bus will use the `PIN_SCREEN_SDA`
// and `PIN_SCREEN_SCL` pins defined below.
#define I2C_USE_DEFAULT_BUS

//--- Input pins ---//
#define PIN_PUMP_BUTTON     D4
#define PIN_MOIST_SENSOR    A0
#define PIN_LUM_SENSOR      A2
#define PIN_ECHO            D8

//--- Output pins ---//
// The relay directly commands the water pump
#define PIN_RELAY           D12
#define PIN_TRIG            D6

//--- I²C pins ---//
// If I2C_USE_DEFAULT_BUS is defined, use different pins to communicate with
// the OLED screen.
#ifndef I2C_USE_DEFAULT_BUS
#define PIN_SCREEN_SDA      D2
#define PIN_SCREEN_SCL      D3
#endif // I2C_USE_DEFAULT_BUS

//--- Miscellaneous constants ---//
// Represents the dryness of the soil. A value greater than
// SOIL_DRYNESS_THRESHOLD indicates the soil is dry. After calibrating the
// sensor, the threshold is considered to be 1900.
#define SOIL_DRYNESS_THRESHOLD  1900
// Since the relation between the luminosity and the value returned by the
// sensor is not linear, a few thresholds are defined rather than taking the
// value as is and map it to a percentage like for the moisture sensor.
#define LIGHT_THRESHOLD_LOW     1500
#define LIGHT_THRESHOLD_GREAT   4000
// Period (in minutes) at which the moisture sensor will be read
#define POLLING_PERIOD_M        30
// Time in ms to wait to avoid bounces on the push button
#define DEBOUNCE_DELAY          50
// OLED screen width, in pixels
#define SCREEN_WIDTH            128         
// OLED screen height, in pixels
#define SCREEN_HEIGHT           64          
// OLED screen address used to communicate over I²C
#define SCREEN_I2C_ADDR         0x3C        
// Distance in mm from the base of the sensor to the bottom of the water tank
#define WATER_SENSOR_DISTANCE   120

#endif // CONFIG_H
