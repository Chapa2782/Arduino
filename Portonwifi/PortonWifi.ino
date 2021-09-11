#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>;
#include <DNSServer.h>
#include <ESP8266WebServer.h >
#include <WiFiManager.h>
#include <AdafruitIO.h>
#include <Adafruit_MQTT.h>
#include <ArduinoHttpClient.h>

#define IO_USERNAME  "Chapa2782"
#define IO_KEY       "aio_bGeY18bBrsZui0vzwzgEq0AfKtz2"


/******************************* WIFI Configuration **************************************/

#define WIFI_SSID ""
#define WIFI_PASS ""


#include "AdafruitIO_WiFi.h"
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// set up the 'command' feed
AdafruitIO_Feed *command = io.feed("porton");

int FINC = D5;   //D5
int FINA = D6;   //D6
int MOTORA = D3;  //D3
int MOTORC = D4;  //D4
int Estado = 0;
String DISPVIS = "SealtoPorton_AAA003";
String DISP = "/Dispositivos/AAA003/";
int numeroDispo = 2;


unsigned long totalTime = 7600;
unsigned long initialTime;
unsigned long currentTime;
unsigned long readTime;
unsigned long readTotalTime = 7000;


#define cParar 0
#define cAbrir 1
#define cCerrar 2
/* COMANDOS
 *  0 = PARAR
 *  1 = ABRIR
 *  2 = CERRAR
 */

#define eCerrado  0
#define eAbriendo  1
#define eAbierto  2
#define eCerrando  3
/*Estados
 * 0 = Cerrado
 * 1 = Abriendo
 * 2 = Abierto
 * 3 = Cerrando */
 
FirebaseData firebaseData;

void setup() {
  pinMode(FINC,INPUT);
  pinMode(FINA,INPUT);
  pinMode(MOTORA, OUTPUT);
  pinMode(MOTORC, OUTPUT);
  digitalWrite(MOTORA,HIGH);
  digitalWrite(MOTORC,HIGH);

  Serial.begin(115200);
  WiFiManager wifiManager;
 
  
  wifiManager.autoConnect("SealtoPorton_AAA003","123456");
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
  Firebase.setInt(firebaseData, DISP + "Estado",0);
  Firebase.setInt(firebaseData, DISP + "Comando",0);
  readTime = millis();
}
 
void loop() {
  currentTime = millis();
  controlFinCar();
  if(FIN_C() || FIN_A()){
    Lectura();
  }
  if(Tempo()){
    Lectura();
  }
  io.run();
}

void Lectura(){
  WiFiManager wifiManager;
  if (Firebase.getInt(firebaseData, DISP + "Comando")) {
    if  (firebaseData.dataType() == "int") {
      int val = firebaseData.intData();
      switch(val){
        case cParar: //Comando Parar
            Parar();
            Firebase.setInt(firebaseData, DISP + "DEV",val);
            CambiarEstado(cParar);
            break;
        case cAbrir: //Comando abrir
            Abrir();
            initialTime = millis();
            Firebase.setInt(firebaseData, DISP + "DEV",val);
            CambiarEstado(cAbrir);
            break;
        case cCerrar: //Comando Cerrar
            Cerrar();
            initialTime = millis();
            Firebase.setInt(firebaseData,DISP + "DEV",val);
            CambiarEstado(cCerrar);
            break;
        case 4:
            Firebase.setInt(firebaseData, DISP + "DEV",val);
            CambiarEstado(cParar);
            wifiManager.resetSettings();
            ESP.reset();
            break;
        }
      }
    }
}
bool Tempo(){
  if((currentTime - readTime) >= readTotalTime && readTime >0){
    readTime = millis();
    return true;
  }else{
    return false;
  }
}
void controlFinCar(){
  if((currentTime - initialTime) >= totalTime && initialTime > 0){
    Parar();
    Firebase.setInt(firebaseData, DISP + "Comando",cParar);
  }
  if(Estado == eAbriendo && FIN_A()){
    Parar();
    Firebase.setInt(firebaseData, DISP + "Comando",cParar);
  }
  if(Estado == eCerrando && FIN_C()){
    Parar();
    Firebase.setInt(firebaseData, DISP + "Comando",cParar);
  }
}
void handleMessage(AdafruitIO_Data *data) {
  int command = data->toInt();
  Serial.print("Comando...");
  Serial.println(command + (numeroDispo * 3));
  if (command == cAbrir + (numeroDispo * 3)){ //light up the LED
    Firebase.setInt(firebaseData, DISP + "Comando",1); 
  }else if(command == cCerrar + (numeroDispo * 3)){
    Firebase.setInt(firebaseData, DISP + "Comando",2); 
  }else if(command == cParar + (numeroDispo * 3)){
    Firebase.setInt(firebaseData, DISP + "Comando",0);
  }
}

bool FIN_C(){
  if(analogRead(FINC) == 1023){
    return true;
  }else{
    return false;
  }
}
bool FIN_A(){
  if(analogRead(FINA) == 1023){
    return true;
  }else{
    return false;
  }
}
void CambiarEstado(int comando){
  switch (comando){
    case cAbrir:
      Estado = eAbriendo;
      Firebase.setInt(firebaseData, DISP + "Estado",eAbriendo);
      break;
    case cCerrar:
      Estado = eCerrando;
      Firebase.setInt(firebaseData, DISP + "Estado",Estado);
      break;
    case cParar:
      if(Estado == eAbriendo){
        Estado = eAbierto;
        Firebase.setInt(firebaseData, DISP + "Estado",Estado);
      }else if(Estado == eCerrando){
        Estado = eCerrado;
        Firebase.setInt(firebaseData, DISP + "Estado",Estado);
      }
  }
}
      

void Abrir(){
  digitalWrite(MOTORC,HIGH);
  digitalWrite(MOTORA,LOW);
}
void Cerrar(){
  digitalWrite(MOTORA,HIGH);
  digitalWrite(MOTORC,LOW);
}
void Parar(){
  digitalWrite(MOTORA,HIGH);
  digitalWrite(MOTORC,HIGH);
  initialTime = 0;
  currentTime = 0;
}


 
 
