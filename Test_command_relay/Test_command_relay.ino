/* 
 * The relay is active low.
 */

/* Output pins */
#define PIN_RELAY 7

volatile byte relayState = HIGH;

void setup() {
  pinMode(PIN_RELAY, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  relayState = !relayState;
  Serial.print("[loop()] Relay State -");
  Serial.println(relayState);
  digitalWrite(PIN_RELAY, relayState);
  delay(10000);
}
