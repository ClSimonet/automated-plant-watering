/*
  test_sleep_mode_with_interrupt
  =====================================
  This code's goal is to demonstrate how to use a pin as an interruption that 
  puts the device in deep sleep, and also using another pin as the external
  wake up source.
  The next step will be to use the same pin for both the interrupt and the
  wake-up source.
  
  This code is under MIT License.
  
  Hardware Connections
  ======================
  Refer to the test_sleep_mode_with_interrupt.png image that can be found in
  the same folder.
  
  NOTE:
  ======
  - Only RTC IO can be used as a source for external wake
  source. They are pins: 0-14 and 17-21, respectively RTC_GPIO0-RTC_GPIO14 and
  RTC_GPIO17-RTC_GPIO21. It is discouraged to use GPIO0 and GPIO3 since they
  are also used for other important functions (such as being strapping pins).

  - The DEBUG_MODE constant allows the user to see debug traces. To do so the
  user must uncomment the "//#define DEBUG_MODE" line below in the
  "Constant definitions" section and open the serial port at 9600 baud in
  the Arduino IDE (Tools > Open Serial Monitor).

  - Using internal pull-down resistor on the wake-up button currently doesn't
  work. The board will wake up immediately after going to sleep. Using an
  external pull-down solves this issue for the moment.

  Date:
  Creation <05/02/2026>
  
  Author:
  Clément Simonet <clement.simonet.pro@gmail.com>
*/

#include "driver/rtc_io.h"

/*--------------------------------
 * Constant definitions
 *--------------------------------*/
#define DEBUG_MODE
//#define USE_INTERNAL_PULLUP_PULLDOWN

/* Input pins */
// Push button that puts the CPU in sleep mode
#define PIN_BUTTON_SLEEP          4
// Wake-up source (only RTC IO are allowed). RTC_GPIO9 <=> Arduino D6
#define PIN_BUTTON_WAKEUP_ESP32   GPIO_NUM_9

/* Output pins */
// Used to light on a LED to prove the CPU is not in sleep mode
#define PIN_LED                 8

/* Constants */
// Time in ms to wait to avoid bounces on the push button
#define DEBOUNCE_DELAY          50

/*--------------------------------
 * Global variables
 *--------------------------------*/
// Used to debounce the push buttons
volatile unsigned long lastEdgeTime = 0;
bool alreadySaidHello = false;

/*--------------------------------
 * Function definitions
 *--------------------------------*/
/// setup()
///
/// Function that will be called only once before loop(). Contains the board,
/// serial port, sleep mode, interrupts and variable setups.
///
/// parameters: none
///
/// returns: void
///
void setup() {
  // Set up the serial port. Used for debugging.
  Serial.begin(9600);
  // Opening the serial port is long, wait a bit.
  delay(1000);
  Serial.println("\n- New execution -\n");

  print_wakeup_reason();

  // Set up pin configurations
  pinMode(PIN_BUTTON_SLEEP, INPUT_PULLDOWN);
  pinMode(PIN_LED, OUTPUT);

  // Set up interruptions
  attachInterrupt(digitalPinToInterrupt(PIN_BUTTON_SLEEP), handleSleepButton, RISING);

  // Initialize GPIO9 to be the wake-up source
  init_rtc();
}

/// loop()
///
/// Function that will be looped over indefinitely. The only work done here is
/// to light an LED as a proof that the program is not in sleep mode.
///
/// parameters: none
///
/// returns: void
///
void loop() {
#ifdef DEBUG_MODE
  Serial.println("[loop()] *Hmmm... did I already say Hi?* " + String(alreadySaidHello));
#endif // DEBUG_MODE
  if (!alreadySaidHello) {
    alreadySaidHello = !alreadySaidHello;
#ifdef DEBUG_MODE
    Serial.println("[loop()] Hello World! - As a proof that I am awake I will light a bulb.");
#endif // DEBUG_MODE
    digitalWrite(PIN_LED, HIGH);
  }
  delay(2000);
}

/// handleSleepButton()
///
/// ISR that is called when the push button that puts the board into sleep mode
/// is pressed or released. The board will go into sleep mode only on rising
/// edges.
///
/// Note: It does maybe too much work for an ISR but it is fine for the moment.
///
/// parameters: none
///
/// returns: void
///
void handleSleepButton() {
  // Fine since it's done as the very first instruction
  unsigned long currTime = millis(); 
  if (currTime - lastEdgeTime > DEBOUNCE_DELAY) {
    // It is a real logical edge, not a bounce
    lastEdgeTime = currTime;  
    // Only continue if the pin is high i.e. if it is a rising edge
    if (digitalRead(PIN_BUTTON_SLEEP) == LOW) return;

#ifdef DEBUG_MODE
    Serial.println("[handleSleepButton()] Button pushed -" + String(currTime) + " ms-");
    Serial.println("[handleSleepButton()] Time to go to bed...");
    // Wait for the messages to be sent before actually going to sleep
    delay(2000);
#endif // DEBUG_MODE
    esp_deep_sleep_start();
  }
}

/// print_wakeup_reason()
///
/// Function called during setup. Intended to be meaningful after waking up
/// from sleep mode. Display what event triggered the boards to come back
/// from sleep mode. Should only display ESP_SLEEP_WAKEUP_EXT0 with the
/// hardware connections and software setup.
///
/// parameters: none
///
/// returns: void
///
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

/// init_rtc()
///
/// Function called during setup. Initialize the ESP32 RTC module in several
/// steps:
///   1- Disconnect all GPIOs from the RTC module
///   2- Connect the GPIO9 to the RTC module and check the if it is now
///   connected
///   3- Set GPIO9 to use internal pull-down (only if 
///   USE_INTERNAL_PULLUP_PULLDOWN is defined)
///   4- Set up EXT0 to wake up when a level HIGH is applied on GPIO9
///
/// parameters: none
///
/// returns: void
///
void init_rtc() {
  esp_err_t esp_err;
  unsigned char i = 0;

  // Unlink all GPIOs from the RTC module, then connect only the GPIO used for the push button. This prevents unsollicited wake-ups if another pin receives a HIGH level.
  for (i = 0; i < 22; i++) {
    if (i == 15 || i == 16) continue;
    esp_err = rtc_gpio_deinit((gpio_num_t)(GPIO_NUM_0 + i));
#ifdef DEBUG_MODE
    if (esp_err == ESP_OK) {
      Serial.println("Successfully unlinked RTC_GPIO" + String(i) + " from RTC.");
    } else {
      Serial.println("Failed to unlink RTC_GPIO" + String(i) + " from RTC.");
    }
  }
#endif // DEBUG_MODE
  rtc_gpio_init(PIN_BUTTON_WAKEUP_ESP32);
#ifdef DEBUG_MODE
  if (esp_err == ESP_OK) {
    Serial.println("Successfully linked RTC_GPIO9 to RTC.");
  } else {
    Serial.println("Failed to link RTC_GPIO9 to RTC.");
  }
#endif // DEBUG_MODE

#ifdef DEBUG_MODE
  // Check if GPIO9 can be used for RTC functions
  if (!rtc_gpio_is_valid_gpio(PIN_BUTTON_WAKEUP_ESP32)) {
    Serial.println("GPIO9 cannot be used for RTC");
  }
#endif // DEBUG_MODE

#ifdef USE_INTERNAL_PULLUP_PULLDOWN
  // Disconnect RTC_GPIO9 from internal pull-up resistor 
  esp_err = rtc_gpio_pullup_dis(PIN_BUTTON_WAKEUP_ESP32);
#ifdef DEBUG_MODE
  if (esp_err == ESP_OK) {
    Serial.println("Successfully disable pullup resistor on RTC_GPIO9");
  } else {
    Serial.println("Failed to disable pullup resistor on RTC_GPIO9");
  }
#endif // DEBUG_MODE

  // Connect RTC_GPIO9 from internal pull-down resistor 
  esp_err = rtc_gpio_pulldown_en(PIN_BUTTON_WAKEUP_ESP32);
#ifdef DEBUG_MODE
  if (esp_err == ESP_OK) {
    Serial.println("Successfully enable pulldown resistor on RTC_GPIO9");
  } else {
    Serial.println("Failed to enable pulldown resistor on RTC_GPIO9");
  }
#endif // DEBUG_MODE
#endif // USE_INTERNAL_PULLUP_PULLDOWN

  // Set wake-up for EXT0 when a level HIGH is applied on RTC_GPIO9
  esp_err = esp_sleep_enable_ext0_wakeup(PIN_BUTTON_WAKEUP_ESP32, 1);
#ifdef DEBUG_MODE
  switch (esp_err) {
    case ESP_OK: 
      Serial.println("Successfully set up the wake up source"); 
      break;
    case ESP_ERR_INVALID_ARG:
      Serial.println("Failed to set up the wake up source: the selected GPIO is not an RTC GPIO, or the mode is invalid");
      break;
    case ESP_ERR_INVALID_STATE:
      Serial.println("Failed to set up the wake up source: wakeup triggers conflict");
      break;
    default:
      Serial.println("Failed to set up the wake up source: unknown reason");
      break;
  }
#endif // DEBUG_MODE
}
