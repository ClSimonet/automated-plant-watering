/* 
  capture.cpp
  =====================================
  Source code of the Capture module of the project.
  This code uses the MCPWM (Motor Control Pulse Width Modulator) Capture module of the ESP32 chip.

  Read the file header in sensors.h for more details.
 */
/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include "config.h"
#include "macros.h"
#include "driver/mcpwm_cap.h"
#include "driver/gpio.h"
#include "esp_private/esp_clk.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*--------------------------------
 * Static function definitions
 *--------------------------------*/
/// echoCallback()
///
/// Function that will be called by the physical ESP32 Capture module when any
/// pulse edge of the capture signal turns active.
///
/// parameters: 
///     - cap_channel: MCPWM capture channel handle
///     - eData: MCPWM capture event data, provided by the driver
///     - user_ctx: data provided when calling
///         mcpwm_capture_channel_register_event_callbacks()
///
/// returns: (bool) true if a high priority task has been waken up by this
///     function
static bool echoCallback(mcpwm_cap_channel_handle_t capChannel,
                         const mcpwm_capture_event_data_t *eData, 
                         void *userData) {
    static uint32_t capValStart = 0;
    static uint32_t capValEnd = 0;
    TaskHandle_t taskToNotify = (TaskHandle_t) userData;
    BaseType_t didNotifiedTaskWakeUp = pdFALSE;

    // Calculate the interval in the ISR to ensure the interval will be always
    // correct even when capture_queue is not handled in time and overflow.
    if (eData->cap_edge == MCPWM_CAP_EDGE_POS) {
        // This is a rising edge. It means the start of the impulse so save the
        // time (i.e. the number of ticks).
        capValStart = eData->cap_value;
        capValEnd = capValStart;
    } else {
        // This is a falling edge. It means the end of the impulse so save the
        // time and notify the number of ticks elapsed during this impulse to
        // the relevant task.
        capValEnd = eData->cap_value;
        uint32_t ticksNum = capValEnd - capValStart;

        // notify the task to calculate the distance
        xTaskNotifyFromISR(taskToNotify, 
                           ticksNum, 
                           eSetValueWithOverwrite, 
                           &didNotifiedTaskWakeUp);
    }

    // Return true if the notified task woke up
    return didNotifiedTaskWakeUp == pdTRUE;
}

/// sendTrigSignal()
///
/// Function that generates a single pulse on the TRIG pin of the HC-SR04. It
/// starts a new measurement.
///
/// parameters: none
///
/// returns: void
static void sendTrigSignal(void)
{
    // The pulse consist of a signal being HIGH for 10 us.
    // Note: FreeRTOS does not allow to set up timing shorter than the
    // millisecond, but in this precise case it is acceptable to use
    // esp_rom_delay_us() because the timing is short enough to not influence
    // too much the FreeRTOS scheduler.
    gpio_set_level(PIN_ESP32_TRIG, 1);
    esp_rom_delay_us(10);
    gpio_set_level(PIN_ESP32_TRIG, 0);
}

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// initCaptureModule()
///
/// Function that initializes and sets up the Capture module on the ESP32
/// microcontroller.
///
/// parameters: none
///
/// returns: void
void initCaptureModule(void)
{
    esp_err_t espStatus = ESP_FAIL;
    TaskHandle_t cur_task = xTaskGetCurrentTaskHandle();
    mcpwm_cap_timer_handle_t capTimer = NULL;
    mcpwm_capture_timer_config_t capTimerCfg;
    mcpwm_cap_channel_handle_t capChannel = NULL;
    mcpwm_capture_channel_config_t capChannelCfg;
    mcpwm_capture_event_callbacks_t capCallback;
    gpio_config_t io_conf;

    D_TRACE("Install the capture timer");
    // MCPWM group ID (arbitrary)
    capTimerCfg.group_id = 0;
    // Use the default MCPWM Capture clock source
    capTimerCfg.clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT;
    espStatus = mcpwm_new_capture_timer(&capTimerCfg, &capTimer);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to install the capture timer-! err: " + String(espStatus));
    }

    D_TRACE("Install a capture channel");
    // The ECHO pin (GPIO 17) receives the signal
    capChannelCfg.gpio_num = PIN_ESP32_ECHO;
    // Don't use a prescaler
    capChannelCfg.prescale = 1;
    // Capture on both edges
    capChannelCfg.flags.pos_edge = true;
    capChannelCfg.flags.neg_edge = true;
    espStatus = mcpwm_new_capture_channel(capTimer, &capChannelCfg, &capChannel);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to install a capture channel-! err: " + String(espStatus));
    }

    // Set up the internal pull-up for the ECHO pin
    espStatus = gpio_set_pull_mode(PIN_ESP32_ECHO, GPIO_PULLUP_ONLY);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to set up the pullup on the ECHO pin-! err: " + String(espStatus));
    }

    D_TRACE("Register capture callback");
    // echoCallback will be called when the signal is received
    capCallback.on_cap = echoCallback;
    espStatus = mcpwm_capture_channel_register_event_callbacks(capChannel, 
                                                               &capCallback,
                                                               cur_task);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to set up the pullup on the ECHO pin-! err: " + String(espStatus));
    }

    D_TRACE("Enable capture channel");
    espStatus = mcpwm_capture_channel_enable(capChannel);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to set up the pullup on the ECHO pin-! err: " + String(espStatus));
    }

    D_TRACE("Configure Trig pin");
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << PIN_ESP32_TRIG;
    espStatus = gpio_config(&io_conf);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to set up TRIG pin-! err: " + String(espStatus));
    }
    // Set LOW by default
    espStatus = gpio_set_level(PIN_ESP32_TRIG, 0);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to set the TRIG pin to LOW at init-! err: " + String(espStatus));
    }

    D_TRACE("Enable and start capture timer");
    espStatus = mcpwm_capture_timer_enable(capTimer);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to enable the capture timer-! err: " + String(espStatus));
    }
    espStatus = mcpwm_capture_timer_start(capTimer);
    if (espStatus != ESP_OK) {
        D_TRACE("!-Failed to start the capture timer-! err: " + String(espStatus));
    }
}

/// measureWaterLevel()
///
/// Function that measures the water level.
/// The same task calling this function must also have called 
/// initCaptureModule() prior to that.
/// Read the function declaration in sensors.h for more details.
///
/// parameters: none
///
/// returns: (float) the distance in mm between the sensor and the water, 0 if
/// the measurement failed
float measureWaterLevel(void)
{
    BaseType_t notifStatus = pdFALSE;
    uint32_t ticksNum = 0;
    float duration = 0;

    // Trigger the sensor to start a new sample
    sendTrigSignal();
    // Wait until the echo is received. The ISR will notify this task.
    notifStatus = xTaskNotifyWait(0x00, 
                                  ULONG_MAX, 
                                  &ticksNum, 
                                  pdMS_TO_TICKS(1000));
    if (notifStatus == pdTRUE) {
        D_TRACE("Notified by the Capture callback");
        duration = ticksNum * (1000000.0 / esp_clk_apb_freq());
        //if (duration > 35000) {
        //    // Out of range
        //    continue;
        //}
        // Convert the duration of the pulse into distance
        return (float) duration / 2 * 0.34;
    } else {
        D_TRACE("Never received the Capture callback notification");
        return 0;
    }
}