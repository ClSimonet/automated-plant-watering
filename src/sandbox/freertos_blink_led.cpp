/* 
  freertos_blink_led.cpp
  =====================================
  Code used to test and play with the real-time OS. 
  It consists in a small application that blinks two LEDs on two different tasks.
  
  Hardware Connections :

  Notes:   

*/

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <Arduino.h>
#include <FreeRTOS.h>
#include "config.h"
#include "macros.h"
//#include "rtos.h"

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
//--- Output pins ---//
#define PIN_LED_BUILTIN         D13
#define PIN_LED_EXTERN          D2 

/*--------------------------------
 * Forward declarations
 *--------------------------------*/
//--- Tasks declarations ---//
void vTaskBlinkBuiltIn(void *pvParameters);
void vTaskBlinkExternal(void *pvParameters);

extern SemaphoreHandle_t serialMutex;

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that is called only once before loop().
///
/// parameters: none
///
/// returns: void
///
void setup() {
  esp_reset_reason_t r = esp_reset_reason();

  // Set up the serial port used to display debug traces
  initLogger(115200);
  D_TRACE("Serial port open");
  // TODO[CS]: Add 3 seconds of delay so I can have the time to manually open
  // the serial port in VS Code. Remove once debug is done.
  delay(3000);

  D_TRACE("Restart reason: " + String(r));

  // Set up the GPIOs
  pinMode(PIN_LED_BUILTIN, OUTPUT);
  pinMode(PIN_LED_EXTERN, OUTPUT);
  D_TRACE("GPIOs initialized");

  // Tasks initialization
  // parameters:
  //   - code routine
  //   - name
  //   - stack size
  //   - input parameters
  //   - priority (the higher the more important)
  //   - task handle (not used)
  xTaskCreate(vTaskBlinkBuiltIn, "Blink Built-in LED", 5000, NULL, 1, NULL);
  xTaskCreate(vTaskBlinkExternal, "Blink External LED", 5000, NULL, 1, NULL);
  D_TRACE("RTOS Tasks created");

  // The Arduino ESP32 frameworks is based on FreeRTOS under the hood thus the
  // FreeRTOS scheduler is already running. Our tasks have been added to it.
  // If called again it would cause a crash.
  //vTaskStartScheduler();
}

/// loop()
///
/// Function that is looped over indefinitely. 
/// Does nothing because everything is done in the tasks.
///
/// parameters: none
///
/// returns: void
///
void loop() {
  // Does nothing. Everything happens in tasks.
}

/// vTaskBlinkBuiltIn()
///
/// FreeRTOS task that blinks the built-in Arduino LED every 500 ms.
///
/// frequency: 
///   500 milliseconds
///
/// parameters:
///   none
///
/// returns:
///   void
///
void vTaskBlinkBuiltIn(void *pvParameters) {
  D_TRACE("vTaskBlinkBuiltIn alive.");
  
  //xSemaphoreTake(serialMutex, pdMS_TO_TICKS(50));
  //Serial.print("[DEBUG] ");
  //Serial.print(__FILENAME__);
  //Serial.print("@");
  //Serial.print(__LINE__);
  //Serial.print(": {");
  //Serial.print(__func__);
  //Serial.print("} ");
  //Serial.println("vTaskBlinkBuiltIn alive.");
  //xSemaphoreGive(serialMutex);
  
  while (1) {
    digitalWrite(PIN_LED_BUILTIN, !digitalRead(PIN_LED_BUILTIN));
    D_TRACE("Going to sleep.");
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

/// vTaskBlinkExternal()
///
/// FreeRTOS task that blinks an external LED every second.
///
/// frequency: 
///   1 seconds
///
/// parameters:
///   none
///
/// returns:
///   void
///
void vTaskBlinkExternal(void *pvParameters) {
  D_TRACE("vTaskBlinkBuiltIn alive.");
  while (1) {
    digitalWrite(PIN_LED_EXTERN, !digitalRead(PIN_LED_EXTERN));
    D_TRACE("Going to sleep.");
    vTaskDelay(pdMS_TO_TICKS(1000));  
  }
}