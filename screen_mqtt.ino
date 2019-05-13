/*** Klient setup ***/
#include <ESP8266WiFi.h> 
#include "Adafruit_MQTT.h" 
#include "Adafruit_MQTT_Client.h"
#include <string.h> 

/*** WiFi Access Point ***/ 
#define WLAN_SSID       "[YOUR SSID]" 
#define WLAN_PASS       "[SSID_PASSWORD]"
#define MQTT_SERVER     "[IP]" // static ip address
#define MQTT_PORT       1883                    
#define MQTT_USERNAME   "" 
#define MQTT_PASSWORD        ""

/*** Global State ***/ 
// Create an ESP8266 WiFiClient class to connect to the MQTT server. 
WiFiClient client; 
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
 
/*** Feeds ***/ 
// Setup a feed called 'esp8266_info_skjerm' for subscribing to changes. 
Adafruit_MQTT_Subscribe info_skjerm = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "/info/esp8266");

/*** Skjerm ***/ 
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h" // Hardware-specific library
#include <SPI.h>
// Koblinger til TFT skjermen.
#define sclk D5   // Don't change, this is the hardware SPI SCLK line
#define mosi D7    // Don't change, this is the hardware SPI MOSI line
#define cs   D3    // Chip select for TFT display, don't change when using F_AS_T
#define dc   D1    // Data/command line, don't change when using F_AS_T
#define rst  5    // Reset, you could connect this to the Arduino reset pin
Adafruit_ILI9341 tft = Adafruit_ILI9341(cs, dc, rst);
unsigned long targetTime = 0;
byte red = 31; 
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;


char *messageRead;
char data[4][6];

/*** DHT sensor ***/
#include "DHT.h"
#define DHTPin D2 
DHT dht;

/*** Sketch Code ***/ 
void setup() { 
 WiFi.mode (WIFI_STA); // turn off AP (Access point) mode
 delay(500);
 Serial.begin(115200); 
 delay(10);
 //TFT skjerm
 tft.begin();
 tft.setRotation(3);  //0 = lodrett, 1 = vannrett
 tft.setTextColor(ILI9341_YELLOW);  //Setter standardfarge på teksten
 targetTime = millis() +1000;
 //DHT-sensor
 dht.setup(DHTPin);
 
 /*** Connect to WiFi access point. ***/
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
 Serial.println("IP address: "); 
 Serial.println(WiFi.localIP());
 // Setup MQTT subscription for info_skjerm feed. 
 mqtt.subscribe(&info_skjerm);
}  

/*** This is where the magic happens ***/
void loop() {
  float temp = dht.getTemperature(); 
  float humi = dht.getHumidity();
  if (targetTime<millis()){
      targetTime = millis() +10000;
      tft.fillScreen(ILI9341_BLACK);    // Fyller skjermen med farge
      tft.drawFastHLine(0,15,110,0xFFE0);     //Setter inn linje til venstre for "Overskrift 1"
      tft.drawFastHLine(210,15,110,0xFFE0);   //Setter inn linje til høyre for "Overskrift 1"
      tft.drawFastHLine(0,115,110,0xFFE0);    //Setter inn linje til venstre for "Overskrift 2"
      tft.drawFastHLine(210,115,110,0xFFE0);  //Setter inn linje til høyre for "Overskrift 2"
      tft.setTextSize(3);                     //Bestemmer tekst str
      /*** Overskrifter ***/
      tft.setTextColor(0xFFE0); //Bestemmer tekstfarge for overskrifter
      tft.setCursor(125,5);
      tft.println("INNE");      //Overskrift 1
      tft.setCursor(135,105);
      tft.println("UTE");       //Overskrift 2
      /*** Statiske tekster ***/
      tft.setTextColor(0x07FF); //Bestemmer tekstfarge for statisk tekst
      tft.setCursor(5,35);
      tft.println("Temp: ");    //Innendørs - Temp
      tft.setCursor(190,35);
      tft.print("C");           //Innendørs - Temp - Benevning
      tft.setCursor(5,70);
      tft.print("Humi: ");      //Innendørs - Humi
      tft.setCursor(190,70);
      tft.print("%");          //Innendørs - Humi - Benevning
      tft.setCursor(5,135);
      tft.println("Temp: ");   //Utendørs - Temp
      tft.setCursor(190,135);
      tft.print("C");          //Utendørs - Temp - Benevning
      tft.setCursor(5,170);
      tft.println("Humi: ");   //Utendørs - Humi
      tft.setCursor(190,170);
      tft.print("%");          //Utendørs - Humi - Benevning
      tft.setCursor(5,205);
      tft.println("Vind: ");   //Utendørs - Vind
      tft.setCursor(190,205);
      tft.print("[m/s]");      //Utendørs - Vind - Benevning
      /*** Dynamiske tekster ***/
      tft.setTextColor(0xFFFF);//Bestemmer tekstfarge for dynamisk tekst
      tft.setCursor(95,35);
      tft.print(temp);         //Innendørs - Temp
      tft.setCursor(95,70);
      tft.print(humi);         //Innendørs - Humi
      tft.setCursor(95,135);
      tft.print(data[0]);      //Utendørs - Temp
      tft.setCursor(95,170);
      tft.print(data[1]);      //Utendørs - Humi
      tft.setCursor(95,205);
      tft.print(data[2]);      //Utendørs - Vind
  }
    
 // MQTT_connect() funskjonen kjører ved opp start og automatisk hvis den blir koblet av). 
 // Funksjonen ligger i vedlagt kodefil: tilkobling
 MQTT_connect(); 
 // Her venter vi på en inncomende 'subscription'
 Adafruit_MQTT_Subscribe *subscription; 
 while ((subscription = mqtt.readSubscription())) { 
   if (subscription == &info_skjerm) { 
     char *message = (char *)info_skjerm.lastread;
     messageRead = message;
       }
   } 
   //Bearbeiding av message fra broker (deler i array med en verdi for hver '/')
   int i =0;
   char * pch;  
   char *pos;
                    
    pch = strtok (messageRead,"/");      
      while(pch != NULL){

      strcpy(data[i], pch);
      Serial.println(data[i]);      
      pch = strtok (NULL, "/"); 
      i++;
     }
 
} 
