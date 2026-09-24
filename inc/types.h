/* 
  types.h
  =====================================
  Header file containing the custom types used by the project.
 */

#ifndef TYPES_H
#define TYPES_H

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>

/*--------------------------------
 * Type definitions
 *--------------------------------*/
// Used to indicate in which context the message must be displayed. It can be:
//   - GENERAL_STATUS: The message contains useful information for the user
//   - SOIL_MOISTURE: The message contains the soil moisture sensor value
//   - LUMINOSITY: The message contains the luminosity sensor value
typedef enum {
  GENERAL_STATUS,
  SOIL_MOISTURE,
  LUMINOSITY,
} oled_msg_ctx;

// Used to describe the current status of the system. It can be:
//   - INIT: The system is currently initializing
//   - SOIL_FINE: The soil moisture has been measured and the soil is fine
//   - SOIL_DRY: The soil moisture has been measured and the soil is dry
//   - WATER_LOW: There is not enough water in the tank. The pump won't be
///      activated.
//   - PUMP_START: The system is currently watering the plants
//   - PUMP_DONE: The system is done watering the plants
typedef enum {
  INIT,
  SOIL_FINE,
  SOIL_DRY,
  WATER_LOW,
  PUMP_START,
  PUMP_DONE,
} gen_status;

// Type passed as parameter to displayOled() in order to print a message
// correctly on the screen.
typedef struct {
  // Indicate the purpose of the message
  oled_msg_ctx ctx;

  // Carry the information to be displayed on screen
  union {
    // Only valid if ctx is GENERAL_STATUS.
    gen_status status;

    // Only valid if ctx is SOIL_MOISTURE or LUMINOSITY.
    uint32_t data;
  } content;
} oled_msg;

#endif // TYPES_H
