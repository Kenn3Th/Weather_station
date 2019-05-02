/************************* Værstasjon setup ***********************************/ 
#include <ESP8266WiFi.h>          //Internett modul/sensor bibliteket
#include "Adafruit_MQTT.h"        //MQTT protokoll bibliotek
#include "Adafruit_MQTT_Client.h" //MQTT klient bibliotek
/************************* WiFi Access Point **********************************/ 
#define WLAN_SSID       "[Your_SSID]"       
#define WLAN_PASS       "[Your_password]" 
#define MQTT_SERVER     "[Your_IP]"          // static ip address
#define MQTT_PORT       1883                    
#define MQTT_USERNAME   "" 
#define MQTT_PASSWORD   "" 
/**************************** Global State ***********************************/ 
// Lager en ESP8266 WiFiClient klasse som kobles til MQTT serveren. 
WiFiClient client; 
// Etter opp MQTT client klassen ved å legge inn WiFi client, MQTT server og login detaljer. 
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD); 
/******************************* Feeds ***************************************/ 
// Definerer en feed kalt "ute"
// Legg merke til at MQTT veier for AIO følger formen: <bruker_navn>/feeds/<feed_navn> 
Adafruit_MQTT_Publish UTE = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/ute/pi");   
/**************************** Sketch Code ************************************/ 

//Henter inn DHT22 sensor bibliotek og definerer pin som sensoren er koblet til
#include "DHT.h"
#define DHTpin D2
DHT dht;

/************************* Vindmåler setup ***********************************/
#define HALLpin D0    //Pinne for vindsensor
#define LEDpin D4     //Pinne for debug-LED

int val;              //Trigger for tidtaker omdreiningstall
int stat=LOW;         //Magnet detektert
int stat2;            //Magnet ikke detektert
float speedk;         //Vindhastighet
int cnt=0;            //Gjeldende antall omdreininger siden siste utregning
int valTime[19];      //Gjeldende tid for omdreining
float radius = 150.0;  //Avstand fra sentrum av spindel til magnet
int interval = 0;     //Interval av cnt rotasjoner
long sum = 0;         //Gjennomsnittlig tid per rotasjon i interval av cnt rotasjoner
long s = 0;           //Total tid for cnt rotasjoner
double rps = 0;       //Antall omdreininger per sekund
double rpm = 0;       //Antall omdreininger per minutt
int total = 0;
long last = 0;
int updint = 10000; //Data sending interval
float speedl[1];
 
void setup() {
 /*WiFi.mode (WIFI_STA); // skrur av AP (Access point) mode*/
 Serial.begin(115200); 
 delay(10); 
 Serial.println(F("RPi-ESP-MQTT")); 
 /********************* Kobler til WiFi access point. ***********************/
 Serial.println(); Serial.println(); 
 Serial.print("Connecting to "); 
 Serial.println(WLAN_SSID); 
 WiFi.begin(WLAN_SSID, WLAN_PASS); 
 while (WiFi.status() != WL_CONNECTED) { 
   delay(500); 
   Serial.print("."); 
 } 
 Serial.println(); 
 Serial.println("WiFi connected"); 
 Serial.println("IP address: "); Serial.println(WiFi.localIP());
 //DHTsensor
 dht.setup(DHTpin);

 /*************** Definerer pinnekonfigurasjon for vindsensor. ***************/
 pinMode(LEDpin, OUTPUT);
 /*attachInterrupt(digitalPinToInterrupt(D0),windGauge, RISING);*/
} 
uint32_t x=0;
 
void loop() {
 /******************** Kjører vindmåling ********************/
  val=digitalRead(HALLpin);
  if(val == 1)
    stat=LOW;
   else {
    stat=HIGH;
    valTime[cnt] = millis();
   }
   digitalWrite(LEDpin,stat); //debugging LED_BUILTIN

  if(stat2!=stat)
   {  //teller når magneten passerer hall-sensoren
     Serial.println(".");
               
     cnt++;
     stat2=stat;
     if (cnt == 19) {
      for (int i = 0; i < 18;i++) {
        interval = valTime[i+1] - valTime[i];
        s = s + interval;
        
      }
      sum = s/18;
      Serial.print("Gjennomsnittstid er: ");
      Serial.print(sum);
      Serial.println(" ms");
     //Beregning av antall omdreininger per sekund:
     double rps = 1000.0/(sum);
     //Beregning av antall omdreininger per minutt:
     double rpm = 60000.0/(sum);
     Serial.print(rps);
     Serial.println(" rps");
     //Omregning til meter per sekund
     double speedk = (2 * 3.1415926536 * (radius/1000) * (double)rps);
     Serial.println("");
     Serial.print(speedk);
     Serial.println(" m/s");
      speedl[total] = speedk;
      total++;
      s = 0;
      cnt = 0;
     }
   }
 /******************** MQTT run ********************/
 // MQTT_connect() funskjonen kjører ved opp start og automatisk hvis den blir koblet av). 
 // Funksjonen ligger i vedlagt kodefil: tilkobling
  MQTT_connect();   

   float temp = dht.getTemperature(), humi = dht.getHumidity();
  if (total == 1){
    Serial.print("Temperatur: ");
    Serial.print(temp);
    Serial.println(" C");
    Serial.print("Fuktighet: ");
    Serial.print(humi);
    Serial.println(" %");
    Serial.print("Vindhastighet: ");
    Serial.print(speedl[0]);
    Serial.println(" m/s");
    total = 0;
  }
  if(millis()-last >= updint){ //Sender data ved gitt tidsintervall
    UTE.publish(temp+500); //Legger til konstantverdi for å skille verdiene fra hverandre i pakkedata
    UTE.publish(humi);
    UTE.publish(speedl[0]+1000);
    Serial.println("Verdier sendt!");
      last = millis();
      Serial.println(last);
  }
}
