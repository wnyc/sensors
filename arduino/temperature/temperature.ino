/** 
 *  Basic ground temperature thermometer.
 *
 * 
 * This document assumes you are working with the 503 thermistor that
 * comes with RadioShack's "Sidekick basic kit for Aruduino"
 * 
 * 
 * Configuration:
 * 
 * This software is preconfigured to work with the GE NTC 503 in the
 * following circuit configuration.  Because the base resistance of the
 * resistor can vary by about 5% there is an optional calibration
 * phase, but we've generally found you can get within a degree or two.
 * 
 * Ground <---/\/\/ 50k ohm /\/\---> A0 <--- /\/\/ NTC 503 /\/\-> +5V
 * 
 * To get the output this design also assumes that you have, a series
 * of 9 LEDS connected to digital pins 2-10 inclusive, wired in series
 * with a resistor between about 300 and 600 ohms.
 * 
 * Optional calibration:
 * 
 * 1. Power down the sensor
 * 2. Fill a glass half full with crused ice and top it off with
 *    water.  Allow to sit for 5 minutes.
 * 3. Immerse the temperature probe into the ice wate mixture
 * 4. Jumper ground to digital I/O #11
 * 5. Power on the sensor with battery power only and leave powered on
 *    until the LEDs stop flashing. This shouldf require about 5 minutes.
 * 6. When the LEDS stop flashing power off the sensor.
 * 7. Remove the wire to digital I/O #11 
 * 
 * To erase the calibration data:
 * 
 * 1. Power donw the sensor
 * 2. Jumper ground to digital I/O #12 
 * 3. Wait until all of the lights are lit
 * 4. Power down the sensor
 * 5. Remove the wire attached to digital I/O #12
 * 
 * How to read the temperature.
 *  
 * There are four distinct steps that go into turning a sensor value
 * into a temperature.  First we have to address the non-linearity of
 * the resistor network that we use to read the sensor.  Our A/D
 * converter values do not linearly translate into resistances.
 * 
 * The above circult uses a pulldown resistor; if we measure the
 * voltage between the thermistor and the resistor it will follow the
 * following formuala:
 * 
 * V0 = V * R  / (R0 + R)
 * 
 * Where:
 * 
 * "V" is the voltage of the circuit.  For our purposes we can use the
 * maximum range of the A/D converter as our unit.
 * R is the resitance of our thermistor
 * R0 is the resistance of our pull 
 * V0 is the value our A/D converter will read
 *  
 * The second step is to apply any calibration factor we might have.
 * THe "base resistance" of the sensor in the Radioshack kit is 5%,
 * this can result in a change of a few degrees, expecially in the
 * higher temperatures.  GE 503's also vary in theor "B" value - that
 * is the rate at which they respond to temperature changes.  We don't
 * at this time try to calibrate that, but might in the future.
 * 
 * 
 * We generate a "correction factor" that we multiple our computed
 * resistance by.  The foruma for this is:
 * 
 * C = Rexp / Rcal
 * 
 * Where:
 * 
 * Rexp is the resistance we expect for our thermistor at 0C
 *  
 * Rcal is the actual resistance resistance we measured during our
 * calibration phase.
 * 
 * So how do we get Rexp?  We can compute it from the Steinhard-Hart
 * equation.  Radioshack doesn't seem to provide a "B" value for this
 * thermistor, but 4150 is pretty typical and seems to work for this
 * kit.
 * 
 * The computation for Rexp is:
 * 
 * Rexp = R0 * exp(B * (1/T - 1/T0))
 * 
 * Where: 
 *   R0: The resistance at T0
 *   T0: The reference temperature of this thermisor's rated
 *       resistance (typically 25C, or 298.15K)
 *   B: The "B" parameter (we're guessing at 4150) 
 *   T: The temperature we want to compute the resistance for
 * 
 * Once we know the current resistance of the sensor we need to
 * convert this into Kelvin.  Again, the formual for this is
 * non-linear.  Because GE provides a B parameter value, we use the B
 * parameter with in the Steinhard-Hart equation as described in the
 * wikipedia (http://en.wikipedia.org/wiki/Thermistor)
 * 
 * So to generate a temperature we compute: 
 * 
 * T = (1 / (1 / T0 + 1 / B * log(R / R0)))
 * 
 * Where: 
 *   T0: The reference "rated" temp of this thermistor (298.15K)
 *   B: The B value (4150)
 *   R: The current resistance (adjusted in our calibration cycle)
 *   R0: The rated resistance at T0
 * 
 * We then translate this into a series of 9 bits that describes the
 * temperature as quarte degrees C above -20C.  This value is shown
 * with a novel encoding that permits the storage of values between
 * [0, 255 + 16] to be stored and decoded within 9 bits without
 * abigioutly in the event of reversal.
 * 
 * The following function is applied to the temperature value: The
 * temperature value is broken into two nibbles, the second bit
 * reversed with a single bit between them indicating which of the
 * nibbles is greater.  This is done to ensure the temepature can be
 * unabigiously decoded whether read from the sensor forward or
 * backward.
 * 
 * Wnyc has a decoder at http://project.wnyc.org/cicadas/
 * 
*/

#include <EEPROM.h>

#define THERMOMETER 0
#define SET_LOW 12


/* The resistance expected of an NTC 503 at 0C
 * This is computed by R = R0 * e**(B*1/T - 1/T0) where:
 * 
 * R0 is the resistance at T0 kelvin
 * B is the NTC B parameter
 * T is the temperature the resistor is actually at
 * R is the anticipated resistance
 */ 

const float expected_resistance = 178747;

/* If you change the pulldown resistors change this. */
const float pulldown_resistance = 50000;

/* If reverse the direction of the resistors and sensor, you have
 * what's called a pull up, not pull down.  Set this to true. 
*/

const bool using_pullups = false;

/* The location in the EEPROM to store calibration data */

const int CALIBRATION_OFFSET = 16;

/* THe number of A/D samples to take to reduce the impact of noise.
 * This is especially important if reading while connected to a USB
 * jack.
 */

const int temp_samples = 16;

/* The max value the A/D converter shows.  For an adrunio this will
 * always be 1023 */

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
  
  
  pinMode(11, INPUT);  
  pinMode(12, INPUT);
  
  /* Turn on the pull up resistors.  See
   * http://arduino.cc/en/Tutorial/DigitalPins
   */

  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
    
}

/* Set one of the bits in the display 
 * 
 * bit: the position from 0..8 inclusive.  Which is right and left?
 * Doesn't matter with our encoding!
 * 
 * value: true is on, false is off.
 */

void setBit(int bit, boolean value) {
  digitalWrite(bit + 2, value ? HIGH : LOW);
}


/* Given a sensor value and the resistance of the pulldown resistor
   compute the sensed resistance */

float compute_resistance(int value, float pulldown_resistance) {
  if (using_pullups)
    value = 1024 - value;  
  return -(pulldown_resistance * (float(value) - 1024.0) / float(value));    
}

/* Given a resistance and R0, T0 and B value describing the NTC thermistor 
 * return the temperature in kelvin.
 */ 
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
  int i;
  for(i=0; i < 9; i++ ) {
    setBit(i, false);
    }
}

/* Write a value between [0..271] inclusive to the display in a way
 * such that reversing the bits doesn't matter.  The decoder is at
 * www.wnyc.org/cicadas and
 * http://github.com/wnyc/sensors/javascript/reversible_bits.js
 */

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
  // Wait until the temperature stablizes and set the eeprom.
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
    for(i=0; i < blink_rate * 3; i++) {
      all_on();
      delay(500/blink_rate);
      all_off();
      delay(500/blink_rate);       
    }
  }
  int final_temp = (temps[offset] + temps[offset - 1] % samples) / 2;
  if ((compute_resistance(final_temp, pulldown_resistance) > 
       expected_resistance * 1.1) || 
      (compute_resistance(final_temp, pulldown_resistance) < 
       expected_resistance * 0.9))
    while(true) 
      error_flash();
  eeprom_write_int((temps[offset] + temps[(offset-1)%samples])/2);
}

void low_set_loop() {
  stablized_temperature_store(1);
  all_on();
  while(1) delay(1000);
}

float compute_temperature(float resistance) {

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
  float temp = compute_temperature(compute_resistance(value, pulldown_resistance));
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

