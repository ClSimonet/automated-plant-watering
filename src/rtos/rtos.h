/* 
  rtos.cpp
  =====================================
  Implementation of the Automated Plant Watering system using a Real-Time Operating System (freeRTOS).

  The system is composed of 6 RTOS task:

    Name: taskReadSoilMoisture
    Frequency: Every 5 minutes.
    Priority: Medium (not critical, yet important for the plant watering logic).
    Estimated duration: < 100 ms.
    Description :
      Reads the value returned by the soil moisture sensor and compute a mean 
      value over the last 5 measurements.
      Output:
        (RTOS queue) Soil moisture value.

    Name: taskReadLuminosity
    Frequency: Every 5 minutes.
    Priority: Medium (a bit lower than taskReadSoilMoisture as there is no
    logic associated to it).
    Estimated duration: < 100 ms.
    Description :
      Reads the value returned by the photoresistor and compute a mean value over
      the last 5 measurements.
      Output:
        (RTOS queue) Luminosity value.

    Name: taskReadWaterLevel
    Frequency: Every 5 minutes.
    Priority: High.
    Estimated duration: < 50 ms.
    Description :
      Reads the value returned by the ultrasonic sensor and compute a mean value
      over the last 5 measurements.
      Output:
        (RTOS queue) Water level value.

    Name: taskDecision
    Frequency: Every 5 minutes.
    Priority: High.
    Estimated duration: < 50 ms.
    Description :
      Receives the value returned by the sensor tasks.
      If the soil moisture value is too low and the water level is ok, then
      enables the water_pump task through a binary semaphore.
      Output:
        (RTOS binary semaphore) Signal sent to the taskWaterPlant task.

    Name: taskWaterPlant
    Frequency: On event.
    Priority: Very High.
    Estimated duration: > 5000 ms.
    Description :
      Receives a signal from taskDecision to activate or deactivate the pump.
      If requested, the pump is activated for 5 seconds.
      Output:
        Activation/deactivation of the water pump.

    Name: taskDisplayOled
    Frequency: Every second.
    Priority: Very Low.
    Estimated duration: < 5000 ms.
    Description :
      Receives a information from different RTOS queues and display them on an
      OLED screen.

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
*/
#ifndef RTOS_H
#define RTOS_H

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
//--- FreeRTOS time translations ---//
// TODO[CS]: Clean unused delays constants
#define DELAY_500MS       pdMS_TO_TICKS(500)
#define DELAY_1S          pdMS_TO_TICKS(1000)
#define DELAY_3S          pdMS_TO_TICKS(3000)
#define DELAY_5S          pdMS_TO_TICKS(5000)
#define DELAY_8S          pdMS_TO_TICKS(8000)
#define DELAY_9S          pdMS_TO_TICKS(9000)
#define DELAY_10S         pdMS_TO_TICKS(10000)
#define TASK_TIMEOUT_1HR  pdMS_TO_TICKS(3600000)

/*--------------------------------
 * Function declarations
 *--------------------------------*/
/// setupFreeRtos()
///
/// Function that will be called only once before loop(). Contains the board,
/// RTOS (tasks, queues and semaphore), and serial port initialization. It then
/// starts the RTOS Scheduler.
///
/// parameters: none
///
/// returns: void
///
void setupFreeRtos(void);

#endif // RTOS_H
