//
// Basic ground temperature thermometer.
// 
// Configuration:
// 
// Analog input #0 is connected as follows;
// 
// 5v+ <---/\/\/ 5k ohm /\/\---> A0 <--- /\/\/ thermistor/\/\-> Ground 
// 
// The thermometer has two calibration phases.  They are performed as follows:
//
// 1. Power down the sensor
// 2. Fill a glass half full with crused ice and top it off with
//    water.  Allow to sit for 5 minutes.
// 3. Immerse the temperature probe into the ice wate mixture
// 4. Jumper 3.3V+ to Digital I/O #12
// 5. Power on the sensor with battery power only and leave powered on
//    until the LEDs stop flashing. This shouldf require about 5 minutes.
// 6. When the LEDS stop flashing power off the sensor.
// 7. Place the sensor under your arm
// 8. Jumper 3.3V+ to Digital I/O #13 instead of #12
// 9. Power on the sensor and wait until the flashing stops.  Thos
//    step should require 5 minutes.
// 10. Power down the sensor
// 11. Remove all jumpers from #12 and #13
//
// Calibration is complete.
//
// Upon starting the sensor temperature is shown as the number of
// quarter degrees above -20C.  This value is shown with a novel
// encoding that permits the storage of values between [0, 239] to be
// stored and decoded within 9 bits without abigioutly in the event of
// reversal.
// 
// The following function is applied to the temperature value: The
// temperature value is broken into two nibbles, the second bit
// reversed with a single bit between them indicating which of the
// nibbles is greater.  This is done to ensure the temepature can be
// unabigiously decoded whether read from the sensor forward or
// backward.
// 
// Prior to display the following function is applied to the
// temperatuer to ensure the upper and lower nibbles are never the
// same:
//
// F(t) = (t+16) * 17 / 16 - 16
//
// To deploy your sensor bury your thermocouple 8 inches under the
// groun in an area of partial share near a tree.  Connect your
// thermometer, power on and record the bit pattern into the WNYC
// Cicada page.


#include <EEPROM.h>

#define THERMOMETER 0
#define SET_LOW 12
#define SET_HIGH 13 

void setup()
{
  pinMode(THERMOMETER, INPUT);
  
  for(i=2;i<11;i++) 
    pinMode(i, OUTPUT);

  pinMode(12, INPUT);
  pinmode(13, INPUT);
}

void setBit(int bit, boolean value) {
  digitalWrite(bit + 2, value ? HIGH | LOW);
}

void all_on() {
  // Set all LEDs on.
  int i;
  for(i=0; i < 9;i++) {
    setBit(i, True);
  }
}

void all_off() {
  // Set all LEDs off.
  int i;
  for(i=0; i < 9; i++ ) {
    setBit(i, False);
    }
}

void write(unsigned byte i) {
  unsigned byte display = 1;
  if (i > 239)
    return all_on();

  i = (i+16) * 17 / 16 - 16

    setBit(0, display & 1);
    setBit(1, display & 2);
    setBit(2, display & 4);
    setBit(3, display & 8);
    setBit(4, ((i >> 4) & 0x0f) > ( i & 0x0f));
    setBit(5, display & 128);
    setBit(6, display & 64);
    setBit(7, display & 32);
    setBit(8, display & 16);
  }

void eeprom_write_int(int o, int i) {
  // Write a signed integer into the EEPROM
  EEPROM.write(o, (i >> 8) & 0x3f);
  EEPROM.write(o+1, i & 0xff);
}

int eeprom_read_int(int i) {
  // Read a signed integer from the EEPROM
  return EEPROM.read(i) << 8 | EEPROM.read(i+1);
}


int measure_temperature() {
  int i;
  sum = 0;
  for(i=0;i<16;i++)
    sum += analogRead(SENSOR);
  sum /= 16;
  return sum;
}

void stablized_temperature_store(int eeprom, int blink_rate) {
  // Wait until the temperaeture stablizes and set the eeprom.
  const int samples=30;
  const int threshold=5;
  int temps[samples]; 
  int i, minimum, maximum,offset;

  for(i=0;i<samples;i+=2)
    temps[i] = 0;
  for(i=1;i<samples;i+=2)
    temps[i] = 1023;

  offset=0;
  minimum = 0;
  maximum = 1023;
  while ((maximum - minimum) > threshold) {
    minimum = temps[0];
    maximum = temps[0];
    for(i=1; i < samples; i++) {
      if (temps[i] < minimum) 
	minimum = temps[i];
      if (temps[i] > maximum) 
	maximum = temps[i];
    }
    temps[offset] = measure_temperature();
    offset = (offset + 1 ) % samples;
    for(i=0;i<blink_rate*3;i++) {
      all_on();
      delay(500/blink_rate);
      all_off();
      delay(500/blink_rate);       
    }
  }
  eeprom_write_int(eeprom, (temps[offset] + temps[(offset-1)%samples])/2);
}

void low_set_loop() {
  stablized_temperature_store("Establishing low temp:", 16, 1);
  all_on();
  while(1) delay(1000);
}

void high_set_loop() {
  stablized_temperature_store("Establishing high temp:", 18, 2);
  all_on();
  while(1) delay(1000);
}


float temp_as_k(int value) {
  
  Serial.print("Low level : ");
  Serial.println(eeprom_read_int(16));
  Serial.print("High level: ");
  Serial.println(eeprom_read_int(18));
  Serial.println(value);
  float temp = value - eeprom_read_int(16);
  Serial.println(temp);
  temp *= 36.5 / float(eeprom_read_int(18) - eeprom_read_int(16));
  temp += 273.15;
  Serial.println(temp);
  return temp;
}
void loop_main() {
  float temp;
  int i;
  int yes=0;
  int no=0;
  int listen_count;


  if ((EEPROM.read(16) == 255 && EEPROM.read(17)) ||
      (EEPROM.read(18) == 255 && EEPROM.read(19)) ) {
    digitalWrite(LED, HIGH);
       
    if (EEPROM.read(16) == 255)
      Serial.println("Low temperature not yet calibrated.  Stick the termocouple in a glass of ice water, jumper pin 10 to 3.3v and power cycle\n");
      
    if (EEPROM.read(18) == 255)
      Serial.println("High temperature not yet calibrated.  Stick the thermocouple under your arm, jumper pin 11 to 3.3v and power cycle\n");
    all_on();
    delay(500);
    all_off();
    delay(500);
    return; 
  }
    
  // Our LED array can emit values in the range [0, 239].  Our
  // temperature will be in units of 0.25 celcius with zero and -20c.  This
  // will give us a range from [-20, 39.75] celcius.
  write_int((temp_as_k(measure_temperature()) + 20.0) / 4.0);
}

void loop() {
  if (digitalRead(12))
    return low_set_loop();
 
  if (digitalRead(13))
    return high_set_loop();
 
  loop_main();
}

