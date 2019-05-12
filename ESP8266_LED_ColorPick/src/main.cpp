#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WiFiManagerByWeldy.h>
#include <DNSServer.h>
    // color swirl! connect an RGB LED to the PWM pins as indicated
    // in the #defines
    // public domain, enjoy!
     
#define REDPIN 13
#define GREENPIN 12
#define BLUEPIN 14
     
#define FADESPEED 5     // make this higher to slow down

unsigned long previousLoopMillis = 0;

int limiteTemperature = 0; //temperature limite établie du côté web serveur

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(82);
   

/*--------------------------------------------------------------------------------
Fonction de programmation OTA
--------------------------------------------------------------------------------*/
void confOTA() {
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("ESP8266_LEDstrip");

  ArduinoOTA.onStart([]() {
    Serial.println("/!\\ Maj OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n/!\\ MaJ terminee");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progression: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}



bool checkSpace(){
  File file = SPIFFS.open("/save.csv", "r");
 
  Serial.println("File Content:");
  uint8_t count = 0;
  String save[] = {};
  /*while(file.available()){
    String part = file.readStringUntil(';');
    Serial.println(part);
    count++;
    save[count] = part;
    if(count >= 4){
      Serial.println(save[0]);
    }
  }*/
  file.close();

  return 1;
}
/*--------------------------------------------------------------------------------
Fonction qui traite les requêtes websocket arrivant depuis le serveur web.
--------------------------------------------------------------------------------*/
void addData(uint16_t couleur, uint8_t * couleurSave){
  uint16_t save = (uint16_t) strtol((const char *) &couleurSave[2], NULL, 10);
  File f = SPIFFS.open("/save.csv", "a+");
  if (!f) {
    Serial.println("erreur ouverture fichier!");
  }else if(checkSpace()){
    if(couleur == 'R'){
        char buffred[16];
        sprintf(buffred,"%d,",save);
        Serial.println(buffred);
        f.print(buffred);
    }if(couleur == 'G'){
        char buffgreen[16];
        sprintf(buffgreen,"%d,",save);
        Serial.println(buffgreen);
        f.print(buffgreen);
        
    }if(couleur == 'B'){
        char buffblue[16];
        sprintf(buffblue,"%d;",save);
        Serial.println(buffblue);
        f.print(buffblue);
    }
    f.close();
  }
}
/*--------------------------------------------------------------------------------
Fonction qui traite les requêtes websocket arrivant depuis le serveur web.
--------------------------------------------------------------------------------*/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if(type == WStype_TEXT){
    uint16_t couleur = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
    if(payload[0] == 'R'){
      analogWrite(REDPIN, couleur);
      delay(FADESPEED);

      Serial.print("rouge = ");
      Serial.println(couleur);
    }
    if(payload[0] == 'G'){
      analogWrite(GREENPIN, couleur);
      delay(FADESPEED);

      Serial.print("vert = ");
      Serial.println(couleur);
    }
    if(payload[0] == 'B'){
      analogWrite(BLUEPIN, couleur);
      delay(FADESPEED);

      Serial.print("bleu = ");
      Serial.println(couleur);
    }
    if(payload[0] =='s'){
      addData(payload[1],payload);
    }
  }

}

void setup() {

  Serial.begin(115200);

  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  

    //WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("Webportail");
  Serial.println("connected...yeey :)");

  if (!MDNS.begin("LEDstrip")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  confOTA();

   if(!SPIFFS.begin()) {
  Serial.println("Erreur initialisation SPIFFS");
  }



  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/js", SPIFFS, "/js");
  server.serveStatic("/style", SPIFFS, "/style");
  server.serveStatic("/save.csv", SPIFFS, "/save.csv");

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent); //Fonction de callback si l'on reçois un message du Webserveur

}


        
void loop() {
  ArduinoOTA.handle(); //Appel de la fonction qui gère OTA
  webSocket.loop(); //Appel de la fonction qui gère la communication avec le websocket
  server.handleClient(); //Appel de la fonctino qui gère la communication avec le serveur web
}