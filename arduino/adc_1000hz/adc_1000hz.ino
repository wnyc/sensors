#define SENSOR 0
long t;
void setup() {
 Serial.begin(115200);
 pinMode(13, OUTPUT);
}

void loop() {
  t = millis();
  Serial.println(analogRead(SENSOR));
  if (t != millis()) digitalWrite(13, HIGH);
  while (millis()==t);
}
