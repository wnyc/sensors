//
// Basic ground temperature thermometer.
// 
// Configuration:
// 
// Analog input #0 is connected as follows;
// 
// 5v+ <---/\/\/ 50k ohm /\/\---> A0 <--- /\/\/ thermistor/\/\-> Ground 
// 
// The thermometer has two calibration phases.  They are performed as follows:
//
// 1. Power down the sensor
// 2. Fill a glass half full with crused ice and top it off with
//    water.  Allow to sit for 5 minutes.
// 3. Immerse the temperature probe into the ice wate mixture
// 4. Jumper Ground to Digital I/O #11
// 5. Power on the sensor with battery power only and leave powered on
//    until the LEDs stop flashing. This shouldf require about 5 minutes.
// 6. When the LEDS stop flashing power off the sensor.
// 7. Place the sensor under your arm
// 8. Jumper ground to Digital I/O #12 instead of #11
// 9. Power on the sensor and wait until the flashing stops.  Thos
//    step should require 5 minutes.
// 10. Power down the sensor
// 11. Remove all jumpers from #11 and #12
//
// Calibration is complete.
//
// Upon starting the sensor temperature is shown as the number of
// quarter degrees above -20C.  This value is shown with a novel
// encoding that permits the storage of values between [0, 255 + 16] to be
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
// To deploy your sensor bury your thermocouple 8 inches under the
// groun in an area of partial share near a tree.  Connect your
// thermometer, power on and record the bit pattern into the WNYC
// Cicada page.


#include <EEPROM.h>

#define THERMOMETER 0
#define SET_LOW 12
#define SET_HIGH 13 
#define HIGH_TEMP 21 // Armpits are 36.5, but for this demo we use room temp,  70F ~= 21C

void setup()
{
   Serial.begin(19200); 
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
 
  // Can't use pin #13 - when we connect something to it, the "L" led on the audrino lights up. 
  // I don't know what that means :-/ 
  
  pinMode(11, INPUT);  
  pinMode(12, INPUT);
  // Turn on the pull up resistors.  See http://arduino.cc/en/Tutorial/DigitalPins
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
    
}

void setBit(int bit, boolean value) {
  digitalWrite(bit + 2, value ? HIGH : LOW);
}



void all_on() {
  // Set all LEDs on.
  int i;
  for(i=0; i < 9;i++) {
    setBit(i, true);
  }
}

void all_off() {
  // Set all LEDs off.
  int i;
  for(i=0; i < 9; i++ ) {
    setBit(i, false);
    }
}

void write(unsigned int i) {
  Serial.print("Indicated temp: ");
  Serial.println(i);

  if (i > 255 + 16) {
    all_on();
    return ;
  }
  if (i > 255) {
    i -= 255;
    setBit(0, i & 1);
    setBit(1, i & 2);
    setBit(2, i & 4);
    setBit(3, i & 8);
    setBit(4, true);
    setBit(5, i & 1);
    setBit(6, i & 2);
    setBit(7, i & 4);
    setBit(8, i & 8);
    return;
  } 
  
  setBit(0, i & 1);
  setBit(1, i & 2);
  setBit(2, i & 4);
  setBit(3, i & 8);
  setBit(4, ((i >> 4) & 0x0f) < ( i & 0x0f));
  setBit(5, i & 128);
  setBit(6, i & 64);
  setBit(7, i & 32);
  setBit(8, i & 16);
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
  int sum = 0;
  for(i=0;i<16;i++)
    sum += analogRead(THERMOMETER);
  sum /= 16;
  return sum;
}

void stablized_temperature_store(int eeprom, int blink_rate) {
  // Wait until the temperaeture stablizes and set the eeprom.
  const int samples=30;
  const int threshold=10;
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
    Serial.print("Current sensor value ");
    Serial.println(temps[offset]);
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
  stablized_temperature_store(16, 1);
  all_on();
  while(1) delay(1000);
}

void high_set_loop() {
  stablized_temperature_store(18, 2);
  all_on();
  while(1) delay(1000);
}


float temp_as_k(int value) {
  
  Serial.print("Low level : ");
  Serial.println(eeprom_read_int(16));
  Serial.print("High level: ");
  Serial.println(eeprom_read_int(18));
  Serial.println("Sensor value: ");
  Serial.println(value);
  float temp = value - eeprom_read_int(16);
  temp *= HIGH_TEMP / float(eeprom_read_int(18) - eeprom_read_int(16));
  temp += 273.15;
  Serial.println("Kelvin: ");
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
         
    if (EEPROM.read(16) == 255)
      Serial.println("Low temperature not yet calibrated.  Stick the termocouple in a glass of ice water, jumper pin 10 to 3.3v and power cycle\n");
      
    if (EEPROM.read(18) == 255)
      Serial.println("High temperature not yet calibrated.  Stick the thermocouple under your arm, jumper pin 11 to 3.3v and power cycle\n");
    all_on();
    delay(500);
    all_off();
    delay(50);
    return; 
  }
    
  // Our LED array can emit values in the range [0, 239].  Our
  // temperature will be in units of 0.25 celcius with zero and -20c.  This
  // will give us a range from [-20, 39.75] celcius.
  write((temp_as_k(measure_temperature()) -273.15 + 20) * 4.0);
  delay(5000);
}

void loop() {
  if (!digitalRead(11)) {
    Serial.println("Trying to set low\n");
    return low_set_loop();
  }
 
  
  if (!digitalRead(12)){
      Serial.println("Trying to set high\n");
    return high_set_loop();
   
  } else Serial.println("Not high\n");
  loop_main();
}

