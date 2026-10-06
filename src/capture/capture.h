/* 
  capture.h
  =====================================
  Source code of the Capture module of the project.
  This code uses the MCPWM (Motor Control Pulse Width Modulator) Capture module of the ESP32. The MCPWM Capture module relies on a dedicated timer to record the time when any pulse edge of the capture signal turns active. From there, it can be convert to another physical quantity such as distance.
  The project uses the MCPWM Capture module to interact with the ultrasonic sensor HC-SR04. The sensor returns an impulse with a width that corresponds to the time travel of the ultrasonic signal, the capture mode is used to measure that width.

  This code does not use the Arduino framework but directly calls the ESP32 APIs instead. It also relies on freeRTOS. It is based on the example provided by Espressif that can be found here:
  https://github.com/espressif/esp-idf/tree/v6.1/examples/peripherals/mcpwm/mcpwm_capture_hc_sr04

 */

#ifndef CAPTURE_H
#define CAPTURE_H

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include "driver/mcpwm_cap.h"
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_log.h"
//#include "esp_private/esp_clk.h"
//#include "driver/mcpwm_cap.h"
//#include "driver/gpio.h"

/*--------------------------------
 * Function declarations
 *--------------------------------*/
/// initCaptureModule()
///
/// Function that initializes and sets up the Capture module on the ESP32
/// microcontroller.
///
/// parameters: none
///
/// returns: void
void initCaptureModule(void);

/// measureWaterLevel()
///
/// Function that measures the water level.
/// First, it send a signal on the TRIG pin of the HC-SR04. Then it waits for
/// the event callback (echoCallback()) to capture the value. Once the event
/// callback notifies by this function, it calculates the distance in cm and
/// infers the water level in cm.
/// The same task calling this function must also have called 
/// initCaptureModule() prior to that.
///
/// parameters: none
///
/// returns: (float) the distance in mm between the sensor and the water, 0 if
/// the measurement failed
float measureWaterLevel(void);

#endif // CAPTURE_H
