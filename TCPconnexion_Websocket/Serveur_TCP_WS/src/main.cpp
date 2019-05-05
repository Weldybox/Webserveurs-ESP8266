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
#include <DNSServer.h>
#include <WiFiManagerByWeldy.h>
#include <FS.h>

float request = 0.0;

//Loop millis
unsigned long previousLoopMillis = 0;

int limiteTemperature = 0; //temperature limite établie du côté web serveur

unsigned long otamillis; //Variable qui va permettre de calculer le temps d'éxecution d'OTA

//Adafruit_BME280 bme; //Objet bme pour gérer la librairie BME
WiFiServer server(82);
IPAddress ip(192, 168, 1,25); 
IPAddress gateway(192,168,0,1);           // gateway of your network
IPAddress subnet(255,255,255,0);

ESP8266WebServer serverWeb;
WebSocketsServer webSocket = WebSocketsServer(81);


/*--------------------------------------------------------------------------------
Page web avec laquelle nous communiquerons avec l'aide d'un webscoket
--------------------------------------------------------------------------------*/
char webpage[] PROGMEM = R"=====(
<html>
<head>
<style>
input[type=range] {
  -webkit-appearance: none;     /*nécessaire pour Chrome */
  padding: 0;                   /* nécessaire pour IE */
  font: inherit;                /* même rendu suivant font document */
  outline: none;
  color: #069;                  /* sert pour couleur de référence, via currentColor, pour le curseur */
  opacity: .8;
  background: #CCC;             /* sert pour couleur de fond de la zone de déplacement */
  box-sizing: border-box;       /* même modèle de boîte pour tous */
  transition: opacity .2s;
  cursor: pointer;
}
</style>
<link href="https://fonts.googleapis.com/css?family=Noto+Sans" rel="stylesheet">
  <script>
    var Socket;
    function init() {
      Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
      Socket.onmessage = function(event){
        document.getElementById("temperature").innerHTML = event.data;
      }
    }
  function sendLimite(){
  Socket.send("#"+document.getElementById("thermostat").value);
}
  </script>
</head>
<body style="font-family: 'Noto Sans', sans-serif;" onload="javascript:init()">
  <div style="font-size:32px;text-align:center;font-weight:bold;">
    <p id="temperature"></p>
  </div>
  <div style="text-align:center;">
    <input type="range" min="0" max="25" value="50" id="thermostat" oninput="sendLimite()" />
  </div>
</body>
</html>
)=====";

/*--------------------------------------------------------------------------------
Fonction de programmation OTA
--------------------------------------------------------------------------------*/
void confOTA() {
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("espOTA");

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
    if(payload[0] == '#'){
      uint16_t temperatureLimite = (uint16_t) strtol((const char *) &payload[1], NULL, 10);
      limiteTemperature = temperatureLimite;
      Serial.print("thermostat = ");
      Serial.println(temperatureLimite);
    }
  }

}

void setup()
{

  /*--------------------------------------------------------------------------------
  Création du lien I2C et déclaration de l'objet BME
  --------------------------------------------------------------------------------*/
  Wire.begin(2, 12);
  Wire.setClock(100000);
  
  /*status = bme.begin(0x76);//L'adresse du couple de connecteur I2C
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
  }*/




  Serial.begin(115200);
  Serial.println();

  WiFi.config(ip, gateway, subnet); 

  WiFiManager wifiManager;
  wifiManager.autoConnect("Webportail");
  Serial.println("connected...yeey :)");

  //SPIFFS.begin();
  if(!SPIFFS.begin()) {
  Serial.println("Erreur initialisation SPIFFS");
}

  if (!MDNS.begin("WeldyTemp")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
Serial.println("mDNS responder started");
    // configuration OTA
  confOTA();

  serverWeb.on("/",[](){
  serverWeb.send_P(200, "text/html", webpage);
  });
  server.begin();
  serverWeb.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent); //Fonction de callback si l'on reçois un message du Webserveur

}

void loop()
{
  WiFiClient client = server.available();
  if (client) {
    if (client.connected()) {
      Serial.println(".");
      request = (client.readStringUntil('\r')).toFloat();    // receives the message from the client
      Serial.print("From client: "); Serial.println(request);
      char buffer[8];
      char* send_msg = dtostrf(request, 2, 4, buffer);
       webSocket.broadcastTXT(send_msg, sizeof(send_msg));
      client.flush();
      client.println("Hi client! No, I am listening.\r"); // sends the answer to the client
    }
    client.stop();                // tarminates the connection with the client
  }

  unsigned long currentLoopMillis = millis();
  if(currentLoopMillis - previousLoopMillis >= 5000){
    if(request < limiteTemperature){
        Serial.println("IL FAIT FROID!!!");
    }
    previousLoopMillis = millis();
  }


  ArduinoOTA.handle(); //Appel de la fonction qui gère OTA
  webSocket.loop(); //Appel de la fonction qui gère la communication avec le websocket

  serverWeb.handleClient(); //Appel de la fonctino qui gère la communication avec le serveur web
}