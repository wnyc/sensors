

#include <WiShield.h>
#include "uip.h"

#include <string.h>

#include "config.h"

#define WIRELESS_MODE_INFRA     1
#define WIRELESS_MODE_ADHOC     2

// Wireless confuiguration parameters ----------------------------------------
unsigned char local_ip[] = {192,168,1,2};       // IP address of WiShield
unsigned char gateway_ip[] = {192,168,1,1};     // router or gateway IP address
unsigned char subnet_mask[] = {255,255,255,0};  // subnet mask for the local network
const prog_char ssid[] PROGMEM = {"ASYNCLABS"};         // max 32 bytes

unsigned char security_type = 0;        // 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2

// WPA/WPA2 passphrase
const prog_char security_passphrase[] PROGMEM = {"12345678"};   // max 64 characters

// WEP 128-bit keys
// sample HEX keys
prog_uchar wep_keys[] PROGMEM = {       0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,   // Key 0
                                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 1
                                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 2
                                                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00    // Key 3
                                                                };

// setup the wireless mode
// infrastructure - connect to AP
// adhoc - connect to another WiFi device
unsigned char wireless_mode = WIRELESS_MODE_INFRA;

unsigned char ssid_len;
unsigned char security_passphrase_len;



#define LED 7
#define SENSOR 0
#define SAMPLES 4000
int i,j;
int yes=0, no=0;
long transmission_delay;
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
  yes=0;
  no=0;

 sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
 
  transmission_delay = millis() + 5 * 1000; // 15 minutes in ms
  while(millis() < transmission_delay) {
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

  digitalWrite(LED, LOW);
  if (((power_7000hz / 10) > power_4000hz) && (power_7000hz > 100000) && (power_4000hz > 1000)) {
     digitalWrite(LED, HIGH); 
     yes += 1;
  } else no += 1;


  Serial.print(power_7000hz);
  Serial.print(":");
  Serial.print(power_4000hz);
  Serial.print(":");
  Serial.print(yes);
  Serial.print("/");
  Serial.println(no);
  freq = SAMPLES / start;
  delay(500);
  }
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
  
  Serial.print("Temperature: ");
  Serial.print(sum);
  Serial.println("C");

{
        uip_ipaddr_t addr;
        struct uip_udp_conn *c;
        int temp = sum * 10;
        char str[50];
        uip_ipaddr(&addr, 209,20,80,141);
        c = uip_udp_new(&addr, (u16_t)HTONS(17));
       
        sprintf(str,"cicada\t1\t%dt\%d\t%d\n",temp, yes, no); 

        memcpy(uip_appdata, str, strlen(str));
        uip_send((const char*)uip_appdata, strlen(str));
        uip_udp_remove(c);
}
}
