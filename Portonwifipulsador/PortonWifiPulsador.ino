#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>;
#include <DNSServer.h>
#include <ESP8266WebServer.h >
#include <WiFiManager.h>
#include <AdafruitIO.h>
#include <Adafruit_MQTT.h>
#include <ArduinoHttpClient.h>

#define IO_USERNAME    "Chapa2782"
#define IO_KEY         "aio_uAPD14CCOYnGIu0WwhRJvswNrqSN"


/******************************* WIFI Configuration **************************************/

#define WIFI_SSID ""
#define WIFI_PASS ""


#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// set up the 'command' feed
AdafruitIO_Feed *command = io.feed("porton");


int MOTORC = D0;  //D4

String DISPVIS = "SealtoPorton_AAA002";
String DISP = "/Dispositivos/AAA002/";
int numeroDispo = 1;
int Estado = 0;



#define cParar 0
#define cAbrir 1
#define cCerrar 2
/* COMANDOS
 *  0 = PARAR
 *  1 = ABRIR
 *  2 = CERRAR
 */


FirebaseData firebaseData;

void setup() {
  pinMode(MOTORC, OUTPUT);
  digitalWrite(MOTORC,HIGH);

  Serial.begin(115200);
  WiFiManager wifiManager;
 
  
  wifiManager.autoConnect("SealtoPorton_AAA002","123456");
  //wifiManager.resetSettings();
  
  command->onMessage(handleMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println(io.statusText());
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
   
  Firebase.begin("https://sealtoporton.firebaseio.com/", "GEASjeDTH6KLc0wAf7jLNjEAugGIMfKi2ZJWBTQ3");
  Firebase.reconnectWiFi(true);

  
  //firebaseData.setBSSLBufferSize(5,5);

  //firebaseData.setResponseSize(1024);

  //6. Optional, set number of error retry
 Firebase.setMaxRetry(firebaseData, 0);

//7. Optional, set number of error resumable queues
Firebase.setMaxErrorQueue(firebaseData, 0);

  Firebase.setReadTimeout(firebaseData, 1000 * 60 * 15); // estaba en 60 * 1000
  
  //Firebase.setwriteSizeLimit(firebaseData, "tiny");
  Firebase.setInt(firebaseData, DISP + "Comando",3);
  Firebase.setInt(firebaseData, DISP + "Estado",0);
}
 
void loop() {
 LecturaPulsador();
 io.run();
}
void LecturaPulsador(){
   WiFiManager wifiManager;
  if (Firebase.getInt(firebaseData, DISP + "Comando")) {
    if  (firebaseData.dataType() == "int") {
      int val = firebaseData.intData();
      switch(val){
       case cAbrir: //Comando Parar
             if(Estado == 0){
              Firebase.setInt(firebaseData, DISP + "Estado",2);
            }
            if(Estado == 2){
              Firebase.setInt(firebaseData, DISP + "Estado",0);
            }
            Pulsador();
            Firebase.setInt(firebaseData, DISP + "DEV",val);
           
            break;
       case cCerrar: //Comando Parar
             if(Estado == 0){
              Firebase.setInt(firebaseData, DISP + "Estado",2);
            }
            if(Estado == 2){
              Firebase.setInt(firebaseData, DISP + "Estado",0);
            }
            Pulsador();
            Firebase.setInt(firebaseData, DISP + "DEV",val);
            
            break;
        case 4:
            Firebase.setInt(firebaseData, DISP + "DEV",val);
            wifiManager.resetSettings();
            ESP.reset();
            break;
        }
      }
    }

     if (Firebase.getInt(firebaseData, DISP + "Estado")) {
    if  (firebaseData.dataType() == "int") {
      int val = firebaseData.intData();
        Estado = val;
      }
    }

    
}


void handleMessage(AdafruitIO_Data *data) {
  int command = data->toInt();
  Serial.print("Comando...");
  Serial.println(command + (numeroDispo * 3));
  if (command == cAbrir + (numeroDispo * 3)){ //light up the LED
    Firebase.setInt(firebaseData, DISP + "Comando",1); 
    //Pulsador();
  }else if(command == cCerrar + (numeroDispo * 3)){
    Firebase.setInt(firebaseData, DISP + "Comando",2); 
    //Pulsador();
  }else if(command == cParar + (numeroDispo * 3)){
    //Firebase.setInt(firebaseData, DISP + "Comando",0);
    //Pulsador();
  }
}

void Pulsador(){
  digitalWrite(MOTORC,LOW);
  delay(2000);
  digitalWrite(MOTORC,HIGH_                                                                                                                          );
  Firebase.setInt(firebaseData, DISP + "Comando",3);
}


 
 
