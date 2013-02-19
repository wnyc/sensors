#include <LowPower.h>

#include <EEPROM.h>

#include <SoftwareSerial.h>
#define THERMOMETER 4
#define AMP 5
#define LED 6
#define SENSOR 0
#define SAMPLES 4000
#define AIN 2

int j;
int yes=0, no=0;
float sum = 0;
float icewater = 916;
float bodytemp = (604 - icewater);
float start;
float freq=22000;
float coeff, power_7000hz, power_4000hz, power_10000hz;
float s, s_prev, s_prev2;
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif




SoftwareSerial GPRS(7, 8);
unsigned char buffer[64]; // buffer array for data recieve over serial port
int count=0;     // counter for buffer array 
boolean low_set=false;
boolean high_set = false;


boolean is_radio_on() {
  bool is_on = false;
  long timer = millis();
  GPRS.write("AT+CMGF=1\n");
  for(timer=millis(); timer + 500 > millis();) {
    if (GPRS.available()) {
      is_on = true;
      GPRS.read();
    }
  }
  return is_on;
}
void setup()
{
  GPRS.begin(19200);               // the GPRS baud rate   
  Serial.begin(19200);             // the Serial port of Arduino baud rate.
  pinMode(THERMOMETER, OUTPUT);
  pinMode(AMP, OUTPUT);
 
 
  pinMode(LED, OUTPUT); // Sets digital pin 13 as output
  pinMode(10, INPUT);
  pinMode(11, INPUT); 
  if (digitalRead(10)) {
    Serial.println("Setting low temperature");
    low_set = true;
  }
  if (digitalRead(11)) {
    Serial.println("Setting high temperature");
    high_set = true;
  }
  Serial.println("Starting up!");
 
}

void powerUpOrDown()
{
  Serial.println("Toggling power switch");
  pinMode(9, OUTPUT);
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}
void powerUp() {
  if (!is_radio_on()) powerUpOrDown();
}
void powerDown() {
  if (is_radio_on()) powerUpOrDown();
}
void SendTextMessage(char *msg)
{
  Serial.println("Sending text message");
  GPRS.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
  delay(100);
  GPRS.println("AT + CMGS = \"+16463713426\"");//send sms message, be careful need to add a country code before the cellphone number
  delay(100);
  GPRS.println(msg);//the content of the message
  delay(100);
  GPRS.println((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  GPRS.println();
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

int temps[30];
void stablized_temperature_store(char *desc, int eeprom, int blink_rate) {
 
  int i, minimum, maximum,offset;
  digitalWrite(THERMOMETER, HIGH);
  delay(500);
  for(i=0;i<30;i+=1) {
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
    for(i=1;i<30;i++) {
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

    offset = (offset + 1 ) % 30;
    for(i=0;i<blink_rate*3;i++) {

      digitalWrite(LED, HIGH);
      delay(500/blink_rate);
      digitalWrite(LED, LOW);
      delay(500/blink_rate);       
    }
  }
  eeprom_write_int(eeprom, (temps[offset] + temps[(offset-1)%30])/2);
  digitalWrite(THERMOMETER, LOW);
}

void low_set_loop() {
  Serial.println("low_set_loop();");
  stablized_temperature_store("Establishing low temp:", 16, 1);
  digitalWrite(LED, HIGH);

  while (1)   LowPower.powerDown(SLEEP_8S, ADC_ON, BOD_ON);  

}
void high_set_loop() {
  Serial.println("high_set_loop();");
  stablized_temperature_store("Establishing high temp:", 18, 2);
  digitalWrite(LED, HIGH);
  while (1) {
      LowPower.powerDown(SLEEP_8S, ADC_ON, BOD_ON);  
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
  powerDown();    


  if ((EEPROM.read(16) == 255 && EEPROM.read(17)) ||
      (EEPROM.read(18) == 255 && EEPROM.read(19)) ) {
    digitalWrite(LED, HIGH);
       
    if (EEPROM.read(16) == 255)
      Serial.println("Low temperature not yet calibrated.  Stick the termocouple in a glass of ice water, jumper pin 10 to 3.3v and power cycle\n");
      
    if (EEPROM.read(18) == 255)
      Serial.println("High temperature not yet calibrated.  Stick the thermocouple under your arm, jumper pin 11 to 3.3v and power cycle\n");
    delay(1000);
    return; 
  }
    
  digitalWrite(THERMOMETER, LOW);
  digitalWrite(AMP, HIGH);
  delay(500);

  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  digitalWrite(1, HIGH);
  for(listen_count=0;listen_count < 100; listen_count += 1) {
    Serial.println("Listening");
    coeff = 2 * cos(2*M_PI*7000/freq);

    s = 0;
    s_prev = 0;
    s_prev2 = 0;
    start = float(millis()) / 1000;
    for(i=0; i<SAMPLES; i++) {
      s = float(analogRead(AIN)) + coeff * s_prev - s_prev2;
      s_prev2 = s_prev;
      s_prev = s;
    }
    start = float(millis())/1000 - start;
    power_7000hz = s_prev2*s_prev2+s_prev*s_prev-coeff*s_prev*s_prev2;
    Serial.print("7000hz: ");
    Serial.println(power_7000hz);
   
    s = 0;
    s_prev = 0;
    s_prev2 = 0;

    coeff = 2 * cos(2*M_PI*4000/freq);
    for(i=0; i<SAMPLES; i++) {
      s = float(analogRead(AIN)) + coeff * s_prev - s_prev2;
      s_prev2 = s_prev;
      s_prev = s;
    }

    power_4000hz = s_prev2*s_prev2+s_prev*s_prev-coeff*s_prev*s_prev2;
    Serial.print("4000hz: ");
    Serial.println(power_4000hz);
  

    digitalWrite(LED, LOW);
    if (((power_7000hz / 10) > power_4000hz) && (power_7000hz > 100000) && (power_4000hz > 10000) ) {
      digitalWrite(LED, HIGH); 
      Serial.println("BUGS!!!!!!!!!!!");
      yes += 1;
    } 
    else {
      no += 1; }


    freq = SAMPLES / start;
    Serial.print("freq:");
    Serial.println(freq);
    delay(500);
  
  
  }
  sbi(ADCSRA,ADPS2) ;
  sbi(ADCSRA,ADPS1) ;
  sbi(ADCSRA,ADPS0) ;

  digitalWrite(THERMOMETER, HIGH);
  digitalWrite(AMP, LOW);
  delay(500);

  temp = temp_as_k(measure_temperature(SENSOR));
  
  Serial.print("temp: ");
  Serial.println(temp);
   
  if (1) {
    digitalWrite(THERMOMETER, LOW);
    digitalWrite(AMP, LOW);
    
    powerUp();
    for(i=0;i<10;i++) delay(1000);
    char str[50];
    sprintf(str,"cicada\t1\t%f\t%d\t%d",temp, yes, no); 
    SendTextMessage(str);
    delay(30000);
    powerDown();
    for(i=0;i<7;i++)   LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  } 
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

