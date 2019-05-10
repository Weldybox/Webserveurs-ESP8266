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

/*--------------------------------------------------------------------------------
Fonction qui traite les requêtes websocket arrivant depuis le serveur web.
--------------------------------------------------------------------------------*/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if(type == WStype_TEXT){
    if(payload[0] == 'G'){
      uint16_t vertLevel = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
      Serial.print("niveauVert = ");
      Serial.println(vertLevel);

      analogWrite(REDPIN, vertLevel);
      delay(FADESPEED);

    }else if(payload[0] == 'R'){
      uint16_t redLevel = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
      Serial.print("niveauRouge = ");
      Serial.println(redLevel);

      analogWrite(GREENPIN, redLevel);
      delay(FADESPEED);
    }else if(payload[0] == 'B'){
      uint16_t blueLevel = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
      Serial.print("niveauBleu = ");
      Serial.println(blueLevel);

      analogWrite(BLUEPIN, blueLevel);
      delay(FADESPEED);
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

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent); //Fonction de callback si l'on reçois un message du Webserveur

}


        
void loop() {
  ArduinoOTA.handle(); //Appel de la fonction qui gère OTA
  webSocket.loop(); //Appel de la fonction qui gère la communication avec le websocket
  server.handleClient(); //Appel de la fonctino qui gère la communication avec le serveur web
}