#include <SPI.h>

/*
  Digital Pot Control

  A - connect this to voltage
  W - this is the pot's wiper, which changes when you set it
  B - connect this to ground.

  The circuit:
    A pin connected to +5V
    B pin connected to ground
    An LED and a 220-ohm resisor in series connected from each W pin to ground
    CS - to digital pin 10  (SS pin)
    SDI - to digital pin 11 (MOSI pin)
    CLK - to digital pin 13 (SCK pin)
*/

/*
AD5292 Commands (From Datasheet):
DC - CONT - COMMAND
0) 00 - 0000 - XXXXXXXXXX  Do Nothing
1) 00 - 0001 - NNNNNNNNNN  Write To RDAC
2) 00 - 0010 - XXXXXXXXXX  Read RDAC from SDO in next frame
3) 00 - 0011 - XXXXXXXXXX  Store wiper setting: store RDAC setting to 20-TP memory.
4) 00 - 0100 - XXXXXXXXXX  Reset: refresh RDAC with 20-TP stored value.
5) 00 - 0101 - XXXXXNNNNN  Read contents of 20-TP memory, or status of 20-TP memory, from the SDO output in the next frame.
6) 00 - 0110 - XXXXXXNNNN  Write contents of serial data to control register.
7) 00 - 0111 - XXXXXXXXXX  Read control register from the SDO output in the next frame.
8) 00 - 1000 - XXXXXXXXXN  Software shutdown.
                           N = 0 (normal mode).
                           N = 1 (device placed in shutdown mode).

N: a digit in serial buffer (MSB)
X: Don't Care

000110XXXXXX0110



*/



//
const float target_period = 55; //55 microseconds => 18.18 kHz
const int MIN_LEVEL = 1024;
const int MAX_LEVEL = 2047;
int level = 1025;
int old_level;
// These variables are to count the frequency.
float high_time;
float low_time;
float total_time;
float frequency;
const int FREQ_PIN = 2;

// These pins are for the SPI interface to the potentiometer.
int target_R = 0;
const int SYNC_PIN = 10;
const int dataReadyPin = 6;
const uint16_t RDAC_enable_uncalibrated = 0x1806; // int: 6150;
const uint32_t SPI_FREQ = 200;
const unsigned long LONG_DELAY = 25;
const unsigned long SHRT_DELAY = 1;

void setup() {
  pinMode(FREQ_PIN, INPUT);
  pinMode(SYNC_PIN, OUTPUT);
  pinMode(dataReadyPin, INPUT);
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE1));
  delay(LONG_DELAY);
  Serial.begin(115200);
  sendUint16(RDAC_enable_uncalibrated);
  sendUint16(level);
  delay(LONG_DELAY);
}

void sendUint16(uint16_t value) {
  digitalWrite(SYNC_PIN, LOW);
  delay(SHRT_DELAY);
  SPI.transfer16(value);
  delay(SHRT_DELAY);
  digitalWrite(SYNC_PIN, HIGH);
  delay(SHRT_DELAY);
}

void sendUint8(uint8_t value) {
  digitalWrite(SYNC_PIN, LOW);
  delay(SHRT_DELAY);
  SPI.transfer(value);
  delay(SHRT_DELAY);
  digitalWrite(SYNC_PIN, HIGH);
  delay(LONG_DELAY);
}

void loop_through_Rs() {
  for (level = 1024; level < 2047; level+= 1) {
    sendUint16(level);
    Serial.print(level);
    delay(SHRT_DELAY);
    print_freq();
  }
  for (level; level > 1024; level-= 1) {
    sendUint16(level);
    delay(SHRT_DELAY);
    Serial.print(level);
    print_freq();
  }
}

float get_period(){
  high_time = pulseIn(FREQ_PIN, HIGH);      //read high time
  low_time = pulseIn(FREQ_PIN, LOW);        //read low time
  return (high_time + low_time);
}

void flicker() {
  Serial.println("First");
  sendUint16(1024);
  delay(500);
  Serial.println("Second");
  sendUint16(2047);
  delay(500);
}

void set_resistance(){
  float my_period = get_period();
  //Serial.print("Period: ");
  Serial.print(50*my_period);
  if(my_period > target_period && level > MIN_LEVEL) sendUint16(level-=10);
  if(my_period < target_period && level < MAX_LEVEL) sendUint16(level+=10);
  //Serial.print("Level: ");
  Serial.print(", ");
  Serial.println(level);
  //old_level = level;
  delay(100);
}
void print_freq(){
  float period = get_period();
  float freq = 1000000/period;
  if (Serial.available() > 0){
    Serial.print(period + String("us "));
    Serial.print(" ");
    Serial.print(freq);
    Serial.println(" Hz");
  }
}
void loop() {
  //flicker();
  set_resistance();
  // Serial.println("Enter ressitance (0-20,000 Ohm)");
  // if (Serial.available() > 0) {
  //   // read the incoming byte:
  //   x = Serial.read();
  //   Serial.print("We got ");
  //   Serial.println(x);
  // }
  // int y = map(x, 0, 20000, 1024, 2047);
  // if(y != y_prev){
  //   Serial.print("Sending ");
  //   Serial.println(y);
  //   sendUint16(y);
  //   y_prev = y;
  // } 
  // delay(1000);
}

