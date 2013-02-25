
#include <EEPROM.h>


#define THERMOMETER 0
#define SET_LOW 12
#define SET_HIGH 13 

void setup()
{
  Serial.begin(19200);             // the Serial port of Arduino baud rate.
  pinMode(THERMOMETER, INPUT);

  
  for(i=2;i<11;i++) {
    pinMode(i, OUTPUT);

  if (digitalRead(12)) {
    Serial.println("Setting low temperature");
    low_set = true;
  }
  if (digitalRead(13)) {
    Serial.println("Setting high temperature");
    high_set = true;
  }
  Serial.println("Starting up!");
}
  void setBit(int bit, boolean value) {
    digitalWrite(bit + 2, value ? HIGH | LOW);
  }

  void all_on() {
    int i;
    for(i=0; i < 9;i++) {
      setBit(i, True);
    }
  }

  void all_off() {
    int i;
    for(i=0; i < 9; i++ ) {
      setBit(i, False);
    }
  }

  void write(unsigned byte i) {
    unsigned byte display = 1;
    if (i > 239)
      return all_on();
    
    while(i-=1) {
      for(display +=1;(((display >> 4) & 0x0f) == (display & 0x0f)); display += 1);
    }

    setBit(0, display & 1);
    setBit(1, display & 2);
    setBit(2, display & 4);
    setBit(3, display & 8);
    setBit(4, ((i >> 4) & 0x0f) > ( i & 0x0f));
    setBit(5, display & 16);
    setBit(6, display & 32);
    setBit(7, display & 64);
    setBit(8, display & 128);
  }

void eeprom_write_int(int o, int i) {
  Serial.print("eeprom_write: ");
  Serial.println(i);
  Serial.print("address: ");
  Serial.print(o);
  Serial.print(" gets ");
  Serial.println( (i >> 8) & 0x3f );
  EEPROM.write(o, (i >> 8) & 0x3f);
  Serial.print("address: ");
  Serial.print(o+1);
  Serial.print(" gets ");
  Serial.println( i & 0xff );
  EEPROM.write(o+1, i & 0xff);
}

int eeprom_read_int(int i) {
  return EEPROM.read(i) << 8 | EEPROM.read(i+1);
}

 const int samples=30;
int temps[samples];
void stablized_temperature_store(char *desc, int eeprom, int blink_rate) {
 
  int i, minimum, maximum,offset;
  digitalWrite(THERMOMETER, HIGH);
  delay(500);
  for(i=0;i<samples;i+=1) {
    if (i%2)
      temps[i] = 1;
    else
      temps[i] = 1000;
  }

  offset=0;
  minimum = 0;
  maximum = 1000;
  while ((maximum - minimum) > 5) {
    minimum = temps[0];
    maximum = temps[0];
    for(i=1;i<samples;i++) {
      if (temps[i] < minimum) minimum = temps[i];
      if (temps[i] > maximum) maximum = temps[i];
    }
    temps[offset ] = (analogRead(SENSOR) + analogRead(SENSOR) +
		      analogRead(SENSOR) + analogRead(SENSOR) +
		      analogRead(SENSOR) + analogRead(SENSOR) +
		      analogRead(SENSOR) + analogRead(SENSOR)+
		      analogRead(SENSOR) + analogRead(SENSOR) +
		      analogRead(SENSOR) + analogRead(SENSOR) +
		      analogRead(SENSOR) + analogRead(SENSOR) +
		      analogRead(SENSOR) + analogRead(SENSOR)) / 16;
    Serial.print(desc);
    Serial.print(offset);
    Serial.print(" : ");
    Serial.print(temps[offset]);
    Serial.print(" [");
    Serial.print(minimum);
    Serial.print("-");
    Serial.print(maximum);
    Serial.println("]");

    offset = (offset + 1 ) % samples;
    for(i=0;i<blink_rate*3;i++) {

      digitalWrite(LED, HIGH);
      delay(500/blink_rate);
      digitalWrite(LED, LOW);
      delay(500/blink_rate);       
    }
  }
  eeprom_write_int(eeprom, (temps[offset] + temps[(offset-1)%samples])/2);
  digitalWrite(THERMOMETER, LOW);
}

void low_set_loop() {
  Serial.println("low_set_loop();");
  stablized_temperature_store("Establishing low temp:", 16, 1);
  all_on();
  while(1) delay(1000);



}
void high_set_loop() {
  Serial.println("high_set_loop();");
  stablized_temperature_store("Establishing high temp:", 18, 2);
  while(1) delay(1000);
  all_on();

  }
}


int measure_temperature(int sensor) {
  int i;
  sum = 0;
  for(i=0;i<16;i++)
    sum += analogRead(SENSOR);
  sum /= 16;
  return sum;
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
  kelvin =temp_as_k(measure_temperature(SENSOR));
  (273.15 + 20.0)) / 4.0 ;
  temp = (

  
  Serial.print("temp: ");
  Serial.println(temp);
   

}

void loop() {
  digitalWrite(AMP, LOW);
  digitalWrite(THERMOMETER, LOW);

  if (low_set) {
    return low_set_loop();
  } 
  if (high_set) {
    return high_set_loop();
  }
  loop_main();
}

