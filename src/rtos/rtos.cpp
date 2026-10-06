/* 
  rtos.cpp
  =====================================
  Implementation of the Automated Plant Watering system using a Real-Time Operating System (freeRTOS).

  For more information about the system, please refer to rtos.h.

  Hardware Connections :
    Refer to the rtos.png image that can be found in the same folder.

  Notes:  
    - The pump is only activated for 5s when the soil is considered dry. It may
      not be enough time to humidify the soil correctly. It shouldn't be too
      much impactful because then the soil would be watered again on the next
      polling time until it is considered sufficiently watered. But this 5s
      watering time might be increased in the future if needed.
    
    - The soil moisture sensor value could be refined to be more comprehensive.
      The map() function would be really useful for this. It will be required
      if a screen is added to indicate the moisture percentage for example.

    - Might be neat to add a task dedicated to debug traces: the other tasks
      would push messages on a queue that is read only by this task, and this
      task would be the only one to access the serial port. Might require too
      much memory though. For the moment, only use a mutex when accessing the
      serial port. 
*/

#if ENV_FREERTOS == 1

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <stdint.h>
#include <Arduino.h>
#include "config.h"
#include "macros.h"
#include "types.h"
#include "../sensors/sensors.h"
#include "../display/display.h"
#include "../capture/capture.h"
#include "rtos.h"
// TODO[CS]: Clean these includes if it builds without them
//#include <FreeRTOS.h>
//#include "FreeRTOSConfig.h"
//#include <task.h>
//#include <queue.h>
//#include <Wire.h>
//#include <Adafruit_SSD1306.h>

/*--------------------------------
 * Global variable declarations
 *--------------------------------*/
//--- Queue declarations ---//
QueueHandle_t soilMoistureQueue;
QueueHandle_t luminosityQueue;
QueueHandle_t waterLevelQueue;
QueueHandle_t systemStatusDisplayQueue;
QueueHandle_t soilMoistureDisplayQueue;
QueueHandle_t luminosityDisplayQueue;

//--- Semaphore declarations ---//
// TODO[CS]: -Future Improvement- Use a task notification instead of a binary semaphore to control the pump.
QueueHandle_t pumpControlSem;

/*--------------------------------
 * Forward declarations
 *--------------------------------*/
void taskReadSoilMoisture(void *pvParameters);
void taskReadWaterLevel(void *pvParameters);
void taskReadLuminosity(void *pvParameters);
void taskWaterPlant(void *pvParameters);
void taskDecision(void *pvParameters);
void taskDisplayOled(void *pvParameters);

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setupFreeRtos()
///
/// Function that will be called only once before loop(). Contains the board,
/// and RTOS (tasks, queues and semaphore) initialization. It then starts the
/// RTOS Scheduler.
///
/// parameters:
///     none
///
/// returns:
///     void
///
void setupFreeRtos() {
  BaseType_t opResult = pdFALSE;

  // Set up the serial port used to display debug traces
  initLogger(115200);
  // TODO[CS]: Add 3 seconds of delay so I can have the time to manually open
  // the serial port in VS Code. Remove once debug is done.
  delay(5000);
  D_TRACE("Serial port open");

  // Set up the GPIOs
  pinMode(PIN_MOIST_SENSOR, INPUT);
  pinMode(PIN_LUM_SENSOR, INPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  // Turn off the pump at init (active low)
  digitalWrite(PIN_RELAY, HIGH);
  D_TRACE("GPIOs initialized");

  // Initialize the OLED screen
  if (initOledScreen() == -1) {
    D_TRACE("Failed to initialize the OLED screen");
    // Go into an infinite loop to get the user attention
    while(1);
  }

  // Queue initializations. Each one contains only one element i.e. the last element is always overwritten.
  // parameters:
  //   - number of elements
  //   - size of each element
  soilMoistureQueue = xQueueCreate(1, sizeof(uint16_t));
  luminosityQueue = xQueueCreate(1, sizeof(uint16_t));
  waterLevelQueue = xQueueCreate(1, sizeof(bool));
  systemStatusDisplayQueue = xQueueCreate(1, sizeof(gen_status));
  soilMoistureDisplayQueue = xQueueCreate(1, sizeof(uint16_t));
  luminosityDisplayQueue = xQueueCreate(1, sizeof(uint16_t));
  D_TRACE("RTOS Queues initialized");

  // TODO[CS]: -Future Improvement- Remove once task notification is implemented.
  // Binary Semaphore initialization
  pumpControlSem = xSemaphoreCreateBinary();
  D_TRACE("RTOS Binary Semaphore initialized");
  
  // Tasks initialization
  // parameters:
  //   - code routine
  //   - name
  //   - stack size
  //   - input parameters
  //   - priority (the higher the more important)
  //   - task handle (not used)
  opResult = xTaskCreate(taskWaterPlant, "Water Plant", 4096, NULL, 5, NULL);
  if (opResult != pdPASS) {
    D_TRACE("Failed to create taskWaterPlant");
  }
  opResult = xTaskCreate(taskDecision, "Decision", 4096, NULL, 4, NULL);
  if (opResult != pdPASS) {
    D_TRACE("Failed to create taskDecision");
  }
  opResult = 
    xTaskCreate(taskReadWaterLevel, "Water Level", 4096, NULL, 3, NULL);
  if (opResult != pdPASS) {
    D_TRACE("Failed to create taskReadWaterLevel");
  }
  opResult = 
    xTaskCreate(taskReadSoilMoisture, "Soil Moisture", 4096, NULL, 2, NULL);
  if (opResult != pdPASS) {
    D_TRACE("Failed to create taskReadSoilMoisture");
  }
  opResult = xTaskCreate(taskReadLuminosity, "Luminosity", 4096, NULL, 2, NULL);
  if (opResult != pdPASS) {
    D_TRACE("Failed to create taskReadLuminosity");
  }
  opResult = xTaskCreate(taskDisplayOled, "Display", 4096, NULL, 1, NULL);
  if (opResult != pdPASS) {
    D_TRACE("Failed to create taskDisplayOled");
  }
  D_TRACE("RTOS Tasks created");

  // The Arduino ESP32 frameworks is based on FreeRTOS under the hood thus the
  // FreeRTOS scheduler is already running. Our tasks have been added to it.
  // If called again it would cause a crash.
  //vTaskStartScheduler();
}

/// taskReadSoilMoisture()
///
/// FreeRTOS task that reads soil moisture from an analog sensor, and sends
/// this value to a queue used by taskDecision().
///
/// frequency: 
///   9 seconds
///
/// parameters:
///   none
///
/// returns:
///   void
///
void taskReadSoilMoisture(void *pvParameters) {
  uint16_t val = 0;
  TickType_t lastWakeTime = xTaskGetTickCount();

  D_TRACE("taskReadSoilMoisture alive.");

  while (1) {
    D_TRACE("Waking up.");

    // Read the value on the analog pin
    val = readMoistureSensor();

    // Send the data to the queue, it will be received by taskDecision().
    // xQueueOverwrite() will send data even if the queue is full by
    // overwriting the previous value. 
    // NB: The queue should never be full since the task waiting on it is
    // quicker than this task.
    xQueueOverwrite(soilMoistureQueue, &val);

    D_TRACE("Going to sleep.");
    // Note that vTaskDelayUntil actually updates the lastWakeTime value
    vTaskDelayUntil(&lastWakeTime, DELAY_9S);
  }
}

/// taskReadLuminosity()
///
/// FreeRTOS task that reads luminosity from an analog sensor, and sends this
/// value to a queue used by taskDecision().
///
/// frequency: 
///   9 seconds
///
/// parameters:
///   none
///
/// returns:
///   void
///
void taskReadLuminosity(void *pvParameters) {
  uint16_t val = 0;
  TickType_t lastWakeTime = xTaskGetTickCount();

  D_TRACE("taskReadLuminosity alive.");

  while (1) {
    D_TRACE("Waking up.");

    // Read the value on the analog pin
    val = readLuminositySensor();

    // Send the data to the queue, it will be received by taskDecision().
    // xQueueOverwrite() will send data even if the queue is full by
    // overwriting the previous value. 
    // NB: The queue should never be full since the task waiting on it is
    // quicker than this task.
    xQueueOverwrite(luminosityQueue, &val);

    D_TRACE("Going to sleep.");
    // Note that vTaskDelayUntil actually updates the lastWakeTime value
    vTaskDelayUntil(&lastWakeTime, DELAY_9S);
  }
}

/// taskReadWaterLevel()
///
/// FreeRTOS task that determines the water level in the tank.
/// It reads the distance between the ultrasonic sensor and the water, infers
/// the water level, and sends this value to a queue used by taskDecision().
///
/// frequency: 
///   8 seconds
///
/// parameters:
///   none
///
/// returns:
///   void
///
void taskReadWaterLevel(void *pvParameters) {
  bool isWaterLow = 0;
  float distance = 0;
  float waterLevel = 0;
  TickType_t lastWakeTime = xTaskGetTickCount();

  D_TRACE("taskReadWaterLevel alive.");
  initCaptureModule();

  while (1) {
    D_TRACE("Waking up.");

    distance = measureWaterLevel();
    // Infers the water level from the distance between the sensor and the
    // water, then determine if it the water level is low
    waterLevel = (WATER_SENSOR_DISTANCE - distance);
    isWaterLow = waterLevel < 20 ? true : false;
    D_TRACE("Distance: " + String(distance) + "mm.");
    D_TRACE("Water Level: " + String(waterLevel) + "mm.");
    if (isWaterLow) {
      D_TRACE("(water low)");
    }

    // Send the data to the queue, it will be received by taskDecision().
    // xQueueOverwrite() will send data even if the queue is full by
    // overwriting the previous value. 
    // NB: The queue should never be full since the task waiting on it is
    // quicker than this task.
    xQueueOverwrite(waterLevelQueue, &isWaterLow);

    D_TRACE("Going to sleep.");
    // Note that vTaskDelayUntil actually updates the lastWakeTime value
    vTaskDelayUntil(&lastWakeTime, DELAY_8S);
  }
}

/// taskWaterPlant()
///
/// FreeRTOS task that activates the pump for a few seconds in order to water
/// the plants.
/// It continuously waits for a signal sent by taskDecision() on a queue. 
/// Upon receiving the signal, it activates a relay to turn on the water pump
/// for three seconds before turning it off.
///
/// frequency: 
///   The task is not periodic. It just waits for the signal from
///   taskDecision() i.e. the binary semaphore.
///
/// parameters:
///   none
///
/// returns:
///   void
///
void taskWaterPlant(void *pvParameters) {
  gen_status sysStatus = INIT;

  D_TRACE("taskWaterPlant alive.");

  while (1) {
    D_TRACE("Wait for the signal.");

    // Wait indefinitely for a signal to water the plant
    if (xSemaphoreTake(pumpControlSem,
                       portMAX_DELAY) == pdTRUE)
    {    
      D_TRACE("Signal received.");
      // Turn on the relay to water the plants
      digitalWrite(PIN_RELAY, LOW);

      // The pump is on, send the information to the queue. It will be
      // received by taskDisplayOled(). xQueueOverwrite() will send data
      // even if the queue is full by overwriting the previous value.
      sysStatus = PUMP_START;
      xQueueOverwrite(systemStatusDisplayQueue, &sysStatus);
      D_TRACE("Pump activated. Going to sleep for 5s.");

      // Let the pump open for 5 seconds. Do not use vTaskDelayUntil
      vTaskDelay(DELAY_5S);
      // Turn off the relay
      digitalWrite(PIN_RELAY, HIGH);

      // The pump is off, send the information to the queue. It will be
      // received by taskDisplayOled(). xQueueOverwrite() will send data
      // even if the queue is full by overwriting the previous value.
      sysStatus = PUMP_DONE;
      xQueueOverwrite(systemStatusDisplayQueue, &sysStatus);
      D_TRACE("Pump deactivated.");
    }

    /// The task is not periodic. It just waits for the signal from
    /// taskDecision() i.e. the binary semaphore.
  }
}

/// taskDecision()
///
/// FreeRTOS task that reads the values from the sensors and determines if the
/// plants need to be watered or not.
/// First, it reads the luminosity value sent by taskReadSoilMoisture() on a 
/// queue. Then it reads the soil moisture value sent by taskReadSoilMoisture() 
/// on a queue. Then if the soil is too dry, it reads the water level value sent
/// by taskReadWaterLevel() on a queue. Only if the water level is
/// sufficient, it sends a signal to taskWaterPlant() using a binary semaphore
/// to activate the water pump.
/// Every time the status of the system, the luminosity or the soil moisture
/// changes, the information is sent to a dedicated queue that will be used by
/// taskDisplayOled().
///
/// frequency: 
///   10 seconds
///
/// parameters:
///   none
///
/// returns:
///   void
///
void taskDecision(void *pvParameters) {
  BaseType_t queueRecStatus = errQUEUE_EMPTY;
  uint16_t moistVal = SOIL_DRYNESS_THRESHOLD;
  uint16_t lumVal = LIGHT_THRESHOLD_GREAT;
  bool isWaterLow = 0;
  gen_status sysStatus = INIT;
  TickType_t lastWakeTime = xTaskGetTickCount();

  D_TRACE("taskDecision alive.");

  while (1) {
    D_TRACE("Waking up.");

    // Check if a luminosity value is available
    queueRecStatus = xQueueReceive(luminosityQueue, &lumVal, 0);
    if (queueRecStatus == pdPASS) {
      // Send the new luminosity value to taskDisplayOled() using the dedicated
      // queue. xQueueOverwrite() overwrites the previous value if the queue
      // was full.
      xQueueOverwrite(luminosityDisplayQueue, &lumVal);
    } else if (queueRecStatus == errQUEUE_EMPTY) {
      // TODO[CS]: Decide what indication to send to the user.
      D_TRACE("The luminosity queue is empty.");
    }

    // Check if a soil moisture value is available
    queueRecStatus = xQueueReceive(soilMoistureQueue, &moistVal, 0);
    if (queueRecStatus == pdPASS) {
      // Send the new soil moisture value to taskDisplayOled() using the
      // dedicated queue. xQueueOverwrite() overwrites the previous value if
      // the queue was full.
      xQueueOverwrite(soilMoistureDisplayQueue, &moistVal);

      if (moistVal > SOIL_DRYNESS_THRESHOLD) {
        // The soil is dry. Send the information to taskDisplayOled() using the
        // dedicated queue. xQueueOverwrite() overwrites the previous value if
        // the queue was full.
        sysStatus = SOIL_DRY;
        xQueueOverwrite(systemStatusDisplayQueue, &sysStatus);
        
        // Activate the pump only if there is enough water in the tank. 
        // Wait until a water level information is available in order to
        // be absolutely sure the water level is fine.
        if (xQueueReceive(waterLevelQueue,
                          &isWaterLow,
                          portMAX_DELAY) == pdPASS) {
          if (isWaterLow) {
            // There is not enough water in the tank, the pump cannot be
            // activated. Send the information to taskDisplayOled() using the
            // dedicated queue. xQueueOverwrite() overwrites the previous value
            // if the queue was full.
            sysStatus = WATER_LOW;
            xQueueOverwrite(systemStatusDisplayQueue, &sysStatus);
          } else {
            // Signal taskWaterPlant() to activate the pump using a semaphore
            if (xSemaphoreGive(pumpControlSem) == pdTRUE) {
              D_TRACE("Signaled taskWaterPlant().");
            } else {
              // Failed to signal taskWaterPlant().
              // TODO[CS]: Decide what indication to send to the user.
              D_TRACE("Failed to send the signal to taskWaterPlant(). Might be "
                "because the semaphore has not been taken yet.");
            }
          }
        } else {
          // Failed to read the water level.
          // TODO[CS]: Decide what indication to send to the user.
          D_TRACE("Failed to read the water level.");
        }
      } else {
        // The soil is fine. Send the information to taskDisplayOled() using the
        // dedicated queue. xQueueOverwrite() overwrites the previous value if
        // the queue was full.
        sysStatus = SOIL_FINE;
        xQueueOverwrite(systemStatusDisplayQueue, &sysStatus);
        D_TRACE("Soil Moisture: fine.");
      }
    } else if (queueRecStatus == errQUEUE_EMPTY) {
      // Failed to read the soil moisture.
      // TODO[CS]: Decide what indication to send to the user.
      D_TRACE("The soil moisture queue is empty.");
    }

    D_TRACE("Going to sleep.");
    // Note that vTaskDelayUntil actually updates the lastWakeTime value
    vTaskDelayUntil(&lastWakeTime, DELAY_10S);
  }
}

/// taskDisplayOled()
///
/// FreeRTOS task that receives information about the system and display them
/// on an OLED screen.
/// The information to be displayed is received from taskDecision() in
/// different queues.
/// The task rebuilds the whole message to print on the screen even if only a
/// part of the message has changed (e.g. the soil moisture changed but the
/// luminosity stayed the same). That is because the task calls clearDisplay()
/// which erases all the screen buffer. As a workaround, the task uses
/// persistent String variables that keep the last information displayed and
/// overwrites them with new values only if needed. These persistent String
/// variables are the ones used to rebuild the whole message.
/// Because the luminosity sensor value is not a linear projection of the real
/// life luminosity (x2 on the sensor luminosity does not mean the luminosity
/// has doubled), threshold have to been defined to represent general states of
/// the luminosity received by the plants. There are 2 thresholds that defines
/// 3 states.
/// The soil moisture value is expressed as a percentage where 100% represents
/// a drown soil, and 0% an arid soil.
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
void taskDisplayOled(void *pvParameters) {
  oled_msg msg;
  bool screenUpdate = false;
  uint16_t lumVal = 0;
  uint16_t moistVal = 0;
  uint8_t moistValPerc = 0;
  gen_status sysStatus = INIT;
  TickType_t lastWakeTime = xTaskGetTickCount();

  D_TRACE("taskDisplayOled alive.");

  while (1) {
    D_TRACE("Waking up.");

    screenUpdate = false;

    // Check if a new luminosity value is available
    if (xQueueReceive(luminosityDisplayQueue,
                      &lumVal,
                      0) == pdPASS) {
      // Update the new info (luminosity) to display on the screen
      msg.ctx = LUMINOSITY;
      msg.content.data = lumVal;
      buildOledMessage(msg);
      // There is some new info, the OLED screen will have to be updated
      screenUpdate = true;
    }

    // Check if a new soil moisture value is available
    if (xQueueReceive(soilMoistureDisplayQueue,
                      &moistVal,
                      0) == pdPASS) {
      // Update the new info (soil moisture) to display on the screen
      msg.ctx = SOIL_MOISTURE;
      msg.content.data = moistVal;
      buildOledMessage(msg);
      // There is some new info, the OLED screen will have to be updated
      // TODO[CS] check if screenUpdate is still needed
      screenUpdate = true;
    } 

    // Check if a new status is available
    if (xQueueReceive(systemStatusDisplayQueue,
                      &sysStatus,
                      0) == pdPASS) {
      // Update the new info (general status) to display on the screen
      msg.ctx = GENERAL_STATUS;
      msg.content.status = sysStatus;
      buildOledMessage(msg);
      // There is some new info, the OLED screen will have to be updated
      screenUpdate = true;
    }

    // Time to display the information on screen!
    if (screenUpdate) {
      screenUpdate = false;
      displayOled();
    } else {
      D_TRACE("Nothing new to display.");
    }

    D_TRACE("Going to sleep.");
    // Note that vTaskDelayUntil actually updates the lastWakeTime value
    vTaskDelayUntil(&lastWakeTime, DELAY_1S);
  }
}

#endif // ENV_FREERTOS == 1
