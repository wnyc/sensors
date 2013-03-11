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


// The resistance expected of an NTC 503 at 0C
// This is computed by R = R0 * e**(B*1/T - 1/T0) where:
// 
// R0 is the resistance at T0 kelvin
// B is the NTC B parameter
// T is the temperature the resistor is actually at
// R is the anticipated resistance
// 
// The 503 only has a 5% tolerance.  We store this value and compare it to the value established during calibration to compute the actual R25 for this resistor. 
const float expected_resistance = 178747;
const float pulldown_resistance = 50000;
const bool using_pullups = false;
const int CALIBRATION_OFFSET = 16;
const int temp_samples = 16;
const int AD_MAX = 1023;
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

// v1 = v * r2 / (r1 + r2) 
// r1 = - (r2 v1 - r2 v) / (v1)
float compute_resistance(int value, float pulldown_resistance) {
  if (using_pullups)
    value = 1024 - value;  
  return -(pulldown_resistance * (float(value) - 1024.0) / float(value));    
}


float compute_temperature(float resistance, float R0, float T0, float B) {
   Serial.println("Compute temperature");
   Serial.print("Resistance: ");
   Serial.println(resistance);
   Serial.print("R0: ");
   Serial.println(R0);
   Serial.print("T0: ");
   Serial.println(T0);
   Serial.print("B: ");
   Serial.println(B);
   return 1/(1.0 / T0 + (1.0 / B) * log(resistance / R0));
}

void all_on() {
  // Set all LEDs on.
  int i;
  for(i=0; i < 9;i++) {
    setBit(i, true);
  }
}

void error_flash() {
  int i;
  for(i=0; i<9; i++)
    setBit(i, i & 1);
 
  delay(500);
  for(i=0; i<9; i++)
    setBit(i, ~(i & 1));
  delay(500);
}

void all_off() {
  // Set all LEDs off.
  int i;
  for(i=0; i < 9; i++ ) {
    setBit(i, false);
    }
}

void write(unsigned int i) {
  Serial.print("Temp: ");
  Serial.print((i * 9.0 - 80.0 ) / 20.0);
  Serial.println("F");
  Serial.print("Encoded value: ");
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

void eeprom_write_int(int i) {
  // Write a signed integer into the EEPROM
  EEPROM.write(CALIBRATION_OFFSET, (i >> 8) & 0x3f);
  EEPROM.write(CALIBRATION_OFFSET + 1, i & 0xff);
}

void eeprom_erase_int() { 
  EEPROM.write(CALIBRATION_OFFSET, 0xff);
  EEPROM.write(CALIBRATION_OFFSET + 1, 0xff);
}  

int eeprom_read_int() {
  // Read a signed integer from the EEPROM
  return EEPROM.read(CALIBRATION_OFFSET) << 8 | EEPROM.read(CALIBRATION_OFFSET + 1);
}


int measure_temperature() {
  int i;
  int sum = 0;
  for(i=0;i<temp_samples;i++)
    sum += analogRead(THERMOMETER);
  sum /= temp_samples;
  return sum;
}

void stablized_temperature_store(int blink_rate) {
  // Wait until the temperaeture stablizes and set the eeprom.
  const int samples=30;
  const int threshold=10;
  int temps[samples]; 
  int i, minimum, maximum,offset;
  float expected_resistance = (AD_MAX + 1) * (expected_resistance / (expected_resistance + pulldown_resistance));
  for(i=0;i<samples;i+=2)
    temps[i] = 0;
  for(i=1;i<samples;i+=2)
    temps[i] = AD_MAX;

  offset=0;
  minimum = 0;
  maximum = AD_MAX;
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
  int final_temp = (temps[offset] + temps[offset-1] % samples) / 2;
  if ((compute_resistance(final_temp, pulldown_resistance) > expected_resistance * 1.1) || (compute_resistance(final_temp, pulldown_resistance) < expected_resistance * 0.9))
      while(true) 
      error_flash();
  eeprom_write_int((temps[offset] + temps[(offset-1)%samples])/2);
}

void low_set_loop() {
  stablized_temperature_store(1);
  all_on();
  while(1) delay(1000);
}

// Works for NHQ104B400R5 or R10
float compute_temperature_NHQ104B400(float resistance) {
    // The NHQ104B400R5 that ships with the Radioshacks' Basic Kit for the Ardruni only has a 5% tolerance.  This means our temperature can be off by a little bit.
    // To accomodate this we give the user the option of entering a "calibration cycle" where they establish the actual resistance at 0C.
    // Now to protect against gross miscalibration we discard the results of the calibraiton cycle if the "adjustment ratio" exceeds 5%. 
    const float r25 = 50000;
    if (((eeprom_read_int() != -1)) && 
	((compute_resistance(eeprom_read_int(), pulldown_resistance)) <= expected_resistance * 1.15) && 
        ((compute_resistance(eeprom_read_int(), pulldown_resistance)) >= expected_resistance * 0.90))
      {   
	Serial.print("Calibrated.  Actual R25 value is: ");
	Serial.print(compute_resistance(eeprom_read_int(), pulldown_resistance) * r25 / expected_resistance);
	Serial.println("Ohms");
	
	Serial.print("Eeprom_value: ");
	Serial.println(eeprom_read_int());
	
	return compute_temperature(resistance, 
				   compute_resistance(eeprom_read_int(), pulldown_resistance) * r25 / expected_resistance, 25, 4150);
      }
    Serial.println("Performing uncalibrated thermometer read");
    return compute_temperature(resistance, r25, 25 + 273.15, 4150);
}


float temp_as_k(int value) {
  float temp = compute_temperature_NHQ104B400(compute_resistance(value, pulldown_resistance));
  Serial.print("Resistance: ");
  Serial.println(compute_resistance(value, pulldown_resistance));
  Serial.print("Value: ");
  Serial.println(value);
  Serial.print("Kelvin: ");
  Serial.println(temp);
  return temp;
}

void loop_main() {
  float temp;
  int i;
  int yes=0;
  int no=0;
  int listen_count;


    
  // Our LED array can emit values in the range [0, 239].  Our
  // temperature will be in units of 0.25 celcius with zero and -20c.  This
  // will give us a range from [-20, 39.75] celcius.
  write((temp_as_k(measure_temperature()) -273.15 + 20) * 4.0);
  delay(10000);
}

void loop() {
  if (!digitalRead(11)) {
    Serial.println("Trying to set low\n");
    return low_set_loop();
  }
 
  
  if (!digitalRead(12)){
    Serial.println("Clearing the calibration\n");
    eeprom_erase_int();
    all_on();
    while(true);
  }
   
  loop_main();
}

