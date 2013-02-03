#include<math.h>

#define SENSOR 0
#define DELAY 20
#define FREQ 50
#define INPUT_PIN 1
#define ALARM_PIN 13
const int F = 4;

long t;
int alarm;
int i, j;
float freq[F] = {17.0, 2.0, 3, 5};
float coeff[F];
float s[F], sp[F], sp2[F];
float power[F];
float value;

void setup() {
   Serial.begin(9600);
   for(i=0;i<F;i++) 
      coeff[i] = 2 * cos(2*M_PI*freq[i] / FREQ);
   pinMode(ALARM_PIN, OUTPUT);
}

void loop() {
  for(i=0;i<F;i++) 
   s[i] = sp[i] = sp2[i] = power[i] = 0.0; 
  for(i=0;i<FREQ; i++) {
    t = millis() + DELAY;  
    value = float(analogRead(SENSOR));
    for(j=0;j<F;j++) {
      s[j] = value + coeff[j] * sp[j] - sp2[j];
      sp2[j] = sp[j];
      sp[j] = s[j];
    }
    
    delay(t-millis() - 2);
    while (millis()<t);
  } 
  for(j=0;j<F;j++)
    power[j] = log(abs(1 + sp2[j] * sp[j] + sp[j] * sp[j] - coeff[j] * sp[j] * sp2[j]));
  Serial.println("=======");
  for(j=0;j<F;j++) {
  Serial.print(freq[j]);
  Serial.print(" ");
  Serial.println(power[j]);
  }
  alarm = 0;
  for (j=1;j<F;j++) 
    if (power[j] - 4.5 > power[0])
   {
     alarm += 1;
  } 
  if (alarm >= 2) {
    digitalWrite(ALARM_PIN, HIGH); 
    Serial.print('#');}
  else    
    digitalWrite(ALARM_PIN, LOW); 

}
