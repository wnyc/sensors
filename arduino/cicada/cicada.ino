#define LED 7
#define SENSOR 0
#define SAMPLES 4000
int i;
int bug=0;
float sum = 0;
float icewater = 916;
float bodytemp = 604 - icewater;
float start;
float freq=22000;
float coeff, power_7000hz, power_4000hz, power_2500hz;
float s, s_prev, s_prev2;
// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif



void setup() {
  
 pinMode(LED, OUTPUT); // Sets digital pin 13 as output
 // Note: analog pins always set as inputs
 // open serial port to send data to Mac
 Serial.begin(9600);

}

void loop() {


  sbi(ADCSRA,ADPS2) ;
  sbi(ADCSRA,ADPS1) ;
  sbi(ADCSRA,ADPS0) ;

  sum = 0;
  for(i=0;i<8;i++)
    sum += analogRead(SENSOR);
   sum /= 8;
  // read the value from sensor (0)
                       // returns 0-1023

  sum -= icewater;
  sum *= 37.0;
  sum /= bodytemp;
  // print value to serial port
  

  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;


  coeff = 2 * cos(2*M_PI*7000/freq);

  s = 0;
  s_prev = 0;
  s_prev2 = 0;
  start = float(millis()) / 1000;
  for(i=0; i<SAMPLES; i++) {
    s = float(analogRead(1)) + coeff * s_prev - s_prev2;
    s_prev2 = s_prev;
    s_prev = s;
  }
  start = float(millis())/1000 - start;
  power_7000hz = s_prev2*s_prev2+s_prev*s_prev-coeff*s_prev*s_prev2;
  s = 0;
  s_prev = 0;
  s_prev2 = 0;

  coeff = 2 * cos(2*M_PI*4000/freq);
  for(i=0; i<SAMPLES; i++) {
    s = float(analogRead(1)) + coeff * s_prev - s_prev2;
    s_prev2 = s_prev;
    s_prev = s;
  }

  power_4000hz = s_prev2*s_prev2+s_prev*s_prev-coeff*s_prev*s_prev2;
  s = 0;
  s_prev = 0;
  s_prev2 = 0;
  if (0) {  
  coeff = 2 * cos(2*M_PI*2500/freq);
  for(i=0; i<SAMPLES; i++) {
    s = float(analogRead(1)) + coeff * s_prev - s_prev2;
    s_prev2 = s_prev;
    s_prev = s;
  }
 
  power_2500hz = s_prev2*s_prev2+s_prev*s_prev-coeff*s_prev*s_prev2;
  }
  sbi(ADCSRA,ADPS2) ;
  sbi(ADCSRA,ADPS1) ;
  sbi(ADCSRA,ADPS0) ;

  bug = 0;
  digitalWrite(LED, LOW);
  if (((power_7000hz / 10) > power_4000hz) && (power_7000hz > 100000) && (power_4000hz > 1000)) {
     digitalWrite(LED, HIGH); 
     bug = 1;
  }

  Serial.print(sum);
  Serial.print(":");
  Serial.print(power_7000hz);
  Serial.print(":");
  Serial.print(power_4000hz);
  Serial.print(":");
  Serial.println(bug);
  freq = SAMPLES / start;
}
