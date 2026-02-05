/***************************************************
 * test_read_potentiometer.ino
 *
 * This routine simply reads the values from the potentiometer.
 * The analog pin A7 receives the tension from the potentiometer. It is then
 * sent over the serial port @9600 baud/s.
 * To observe the measured value, the user must open the serial port in the
 * Arduino IDE (Tools > Open Serial Monitor).
 *
 * Notes: 
 * Values go from 0 to 4095. There is a dead zone at the maximum (4095 is
 * returned before the screw is fully turned). There is no dead zone at the
 * minimum.
 */

/* Input pins */
#define PIN_POTENTIOMETER A7

void setup() {
  pinMode(PIN_POTENTIOMETER, INPUT);

  /* Set up the serial port used to display the value on the screen */
  Serial.begin(9600);
}

void loop() {
  /* Read the value on the analog pin 7 */
  int val;
  val = analogRead(PIN_POTENTIOMETER);
  /* Send the value through the serial port */
  Serial.println(val);
  delay(100);
}
