#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>;
#include <DNSServer.h>
#include <ESP8266WebServer.h >
#include <WiFiManager.h>
#include <AdafruitIO.h>
#include <Adafruit_MQTT.h>
#include <ArduinoHttpClient.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define IO_USERNAME  "Chapa2782"
#define IO_KEY       "aio_bGeY18bBrsZui0vzwzgEq0AfKtz2"


/******************************* WIFI Configuration **************************************/

#define WIFI_SSID "CHAPARED"
#define WIFI_PASS "Sealto8227"


#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// set up the 'command' feed
AdafruitIO_Feed *command = io.feed("porton");


int MOTORC = D0;  //D4

String DISPVIS = "SealtoPorton_AAA001";
String DISP = "/Dispositivos/AAA001/";
int numeroDispo = 0;
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

int cont = 0;

void setup() {
  pinMode(MOTORC, OUTPUT);
  digitalWrite(MOTORC,LOW);

  Serial.begin(115200);
  WiFiManager wifiManager;
 
  
  wifiManager.autoConnect("NewSealtoPorton","123456");
  //wifiManager.resetSettings();
  
  io.connect();

   command->onMessage(handleMessage);
  
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  delay(2000);

  command ->get();

  // we are connected
  Serial.println(io.statusText());
 
  //while (WiFi.status() != WL_CONNECTED) {
    //delay(500);
    //Serial.print(".");
  //}
  Serial.println(WiFi.localIP());

   Serial.println("io estatus");
  Serial.println(io.statusText());
   
  Firebase.begin("https://sealtoporton.firebaseio.com/", "GEASjeDTH6KLc0wAf7jLNjEAugGIMfKi2ZJWBTQ3");
  Firebase.reconnectWiFi(true);

  Firebase.setMaxRetry(firebaseData, 3);

  Firebase.setMaxErrorQueue(firebaseData, 10);

  Firebase.setReadTimeout(firebaseData, 1000 * 60 * 15); // estaba en 60 * 1000
  
  Firebase.setInt(firebaseData, DISP + "Comando",3);
  Firebase.setInt(firebaseData, DISP + "Estado",0);
  Serial.println("Comienza inicio con OTA...");


  //Comienza seccion OTA
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
   ArduinoOTA.setHostname("SealtoSoftPorton1");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void loop() {
  ArduinoOTA.handle();
 io.run();
 LecturaPulsador();
}
void LecturaPulsador(){
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
  if(cont == 0){
    Serial.println("Ingreso por primera vez");
    cont  = 1;
    return;
  }
  if (command == cAbrir + (numeroDispo * 3)){ //light up the LED
    //Firebase.setInt(firebaseData, DISP + "Comando",1); 
     if(Estado == 0){
        Firebase.setInt(firebaseData, DISP + "Estado",2);
     }
     if(Estado == 2){
        Firebase.setInt(firebaseData, DISP + "Estado",0);
     }
    Pulsador();
  }else if(command == cCerrar + (numeroDispo * 3)){
    //Firebase.setInt(firebaseData, DISP + "Comando",2); 
    if(Estado == 0){
        Firebase.setInt(firebaseData, DISP + "Estado",2);
     }
     if(Estado == 2){
        Firebase.setInt(firebaseData, DISP + "Estado",0);
     }
    Pulsador();
  }else if(command == cParar + (numeroDispo * 3)){
    //Firebase.setInt(firebaseData, DISP + "Comando",0);
    //Pulsador();
  }
}

void Pulsador(){
  digitalWrite(MOTORC,HIGH);
  delay(2000);
  digitalWrite(MOTORC,LOW);
  
  //Serial.println("Ingreso en pulsador function");        
  Firebase.setInt(firebaseData, DISP + "Comando",3);
}


 
 
