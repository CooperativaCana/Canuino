// This sites tell you all about ESP8266 setup to do before to run this code in arduino.
// https://eldontronics.wordpress.com/2017/08/28/beginning-iot-with-esp8266-01-wifi-module-and-cayenne-iot-platform/

#include <CayenneMQTTESP8266Shield.h>
#include "RTClib.h"
#include <Wire.h>
#include "DHT.h"

char ssid[] = "XXXXX"; /// insert your network name
char wifiPassword[] = "XXXXXX"; /// insert your router/device password

/// generated by cayenne login in arduino Mega2560 with ESP8266, choosing "Arduino Mega > Arduino Mega ESP8266 Wifi"
char username[] = "cd805270-4f14-11e9-b395-f53190c2ab6f";
char password[] = "dcb5452141ec23d783bcc0c0ef46b4a89e57cc5e";
char clientID[] = "cf88f560-f3f4-11e9-a38a-d57172a4b4d4";
#define EspSerial Serial1
ESP8266 wifi(&EspSerial);

//RTC1307 SDA SCL Connection
#if defined(ARDUINO_ARCH_SAMD)
#define Serial SerialUSB
#endif
RTC_DS1307 rtc;

//DHT22 Sensor
#define DHTPIN_veg 2    
#define DHTPIN_flo 3    
#define DHTTYPE DHT22   
DHT dht_flo(DHTPIN_flo, DHTTYPE); 
DHT dht_veg(DHTPIN_veg, DHTTYPE); 

//8 Channel Relay
#define rele_hps 42 
int rele_hpsOut = 42; 
#define rele_cfl 40 
int rele_cflOut = 40; 
#define rele_ctube 44 
int rele_ctubeOut = 44; 
#define rele_bomba_veg 46 
int rele_bomba_vegOut = 46; 
#define rele_valvula 34 
int rele_valvulaOut = 34;
#define rele_ventoinha 32 
int rele_ventoinhaOut = 32; 
#define rele_bomba_bloom 36 
int rele_lampada_vegetativoOut = 36;
#define rele_bomba_grow 38  
int rele_circulacao_vegetativoOut = 38;

//2 x Lights Operation
void timerLampadas () {
  DateTime now = rtc.now();
  if ((now.hour() >= 19 && now.hour() <= 23 ) or (now.hour() >= 0 && now.hour() <= 7 )) 
  {
    digitalWrite(rele_hps, HIGH);
    digitalWrite(rele_ctube, HIGH);
    Cayenne.virtualWrite(5, 1);
  }
  else
  {
    digitalWrite(rele_hps, LOW);
    digitalWrite(rele_ctube, LOW);
    Cayenne.virtualWrite(5, 0);
  }
  
  if ((now.hour() >= 18 && now.hour() <= 23 ) or (now.hour() >= 0 && now.hour() <= 12 )) 
  {
    digitalWrite(rele_cfl, HIGH);
    Cayenne.virtualWrite(8, 1);
  }
  else
  {
    digitalWrite(rele_cfl, LOW);
    Cayenne.virtualWrite(8, 0);
  }
}

//2x Temperature and Humidity sensors to Cayenne Monitoring
void temHumiCayenne() {
  float hum_veg = dht_veg.readHumidity() - 25; // lê e armazena o valor da humidade do sensor da FLORAÇÃO
  float temp_veg = dht_veg.readTemperature(); // lê e armazena o valor da temperatura do sensor da FLORAÇÃO
  float hum_flo = dht_flo.readHumidity() - 25; // lê e armazena o valor da humidade do sensor do VEGETATIVO
  float temp_flo = dht_flo.readTemperature(); // lê e armazena o valor da temperatura do sensor do VEGETATIVO
  Cayenne.virtualWrite(1, temp_veg, TYPE_TEMPERATURE, UNIT_CELSIUS); //// LEITURA DO SENSOR TEMPERATURA CFL
  Cayenne.virtualWrite(2, hum_veg, TYPE_RELATIVE_HUMIDITY, UNIT_PERCENT);//// LEITURA DO SENSOR HUMIDADE CFL
  Cayenne.virtualWrite(3, temp_flo, TYPE_TEMPERATURE, UNIT_CELSIUS); //// LEITURA DO SENSOR TEMPERATURA HPS
  Cayenne.virtualWrite(4, hum_flo, TYPE_RELATIVE_HUMIDITY, UNIT_PERCENT); //// LEITURA DO SENSOR HUMIDADE HPS
}

//2x Capacitive Soil Sensors to Cayenne Monitoring
void soloCayenne() {
  int output_value_cfl;
  int solo_cfl = 0;
  int output_value_hps;
  int solo_hps = 0;
  solo_cfl = analogRead(A14);
  solo_hps = analogRead(A15);
  ///Dry: (520 430]     //// ( 609 - 590 - 560 ) | 0% a 23% SECO / AMARELO
  ///Wet: (430 350]     //// ( 560 - 455 - 350 ) | 24% a 64% HUMIDO / VERDE
  ///Water: (350 260]   //// ( 350 - 305 - 276 ) | 65% a 100% MOLHADO / VERMELHO
  //////map(valor/input a ser mapeado, fromLow, fromHigh, toLow, toHigh)
  output_value_hps = map(solo_hps, 656, 276, 0, 100);
  output_value_cfl = map(solo_cfl, 651, 280, 0, 100);
  Cayenne.virtualWrite(6, output_value_hps); 
  Cayenne.virtualWrite(7, output_value_cfl); 
}
//1x Clock Time Readings to Cayenne Monitoring
void dataCayenne() {
  DateTime now = rtc.now();
  Cayenne.virtualWrite(26, now.hour());
  Cayenne.virtualWrite(27, now.minute());
  Cayenne.virtualWrite(31, now.day());
  Cayenne.virtualWrite(28, now.month());
}

//4x Buttons to action in Cayenne
CAYENNE_IN(V17)
{
  if (getValue.asInt() == 1) {
    digitalWrite(rele_valvula, HIGH);
  }
  else {
    digitalWrite(rele_valvula, LOW);
  }
}

CAYENNE_IN(V18)
{
  if (getValue.asInt() == 1) {
    digitalWrite(rele_bomba_grow, HIGH);
  }
  else {
    digitalWrite(rele_bomba_grow, LOW);
  }
}

CAYENNE_IN(V19)
{
  if (getValue.asInt() == 1) {
    digitalWrite(rele_bomba_bloom, HIGH);
  }
  else {
    digitalWrite(rele_bomba_bloom, LOW);
  }
}

CAYENNE_IN(V20)
{
  if (getValue.asInt() == 1) {
    digitalWrite(rele_bomba_veg, HIGH);
  }
  else {
    digitalWrite(rele_bomba_veg, LOW);
  }
}

void setup()
{
  Serial.begin(9600);
  EspSerial.begin(115200);
  Cayenne.begin(username, password, clientID, wifi, ssid, wifiPassword);
  Wire.begin();
  //rtc.adjust(DateTime(2019, 10, 31, 1, 16, 0));
  dht_veg.begin();
  dht_flo.begin();
  pinMode(32, OUTPUT); 
  pinMode(34, OUTPUT); 
  pinMode(36, OUTPUT); 
  pinMode(38, OUTPUT); 
  pinMode(40, OUTPUT); 
  pinMode(42, OUTPUT); 
  pinMode(44, OUTPUT);
  pinMode(46, OUTPUT); 
}

void loop() {
  timerLampadas ();
  temHumiCayenne();
  soloCayenne();
  dataCayenne();
  Cayenne.loop(); // To upload/download the state of buttons in Cayenne 
}
