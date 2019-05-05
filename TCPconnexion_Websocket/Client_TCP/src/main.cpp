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

#define LED 14

//Loop millis
unsigned long previousLoopMillis = 0;

unsigned long otamillis; //Variable qui va permettre de calculer le temps d'éxecution d'OTA

Adafruit_BME280 bme; //Objet bme pour gérer la librairie BME
IPAddress server(192,168,1,25);
WiFiClient client;

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

void setup()
{
  pinMode(LED,OUTPUT);

  /*--------------------------------------------------------------------------------
  Création du lien I2C et déclaration de l'objet BME
  --------------------------------------------------------------------------------*/
  Wire.begin(2, 12);
  Wire.setClock(100000);
  bool status;
  status = bme.begin(0x76);//L'adresse du couple de connecteur I2C
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
  }


  Serial.begin(115200);
  Serial.println();

  WiFiManager wifiManager;
  wifiManager.autoConnect("Webportail");
  Serial.println("connected...yeey :)");

    // configuration OTA
  confOTA();

}

void loop()
{
    unsigned long currentLoopMillis = millis();
  if(currentLoopMillis - previousLoopMillis >= 5000){
    float temp = bme.readTemperature();
    client.connect(server, 82);   // Connection to the server
    digitalWrite(LED,HIGH);
    client.println(temp);  // sends the message to the server
    String answer = client.readStringUntil('\r');   // receives the answer from the sever
    Serial.println("from server: " + answer);
    client.flush();

    previousLoopMillis = millis();

  }
  digitalWrite(LED,LOW);
  ArduinoOTA.handle(); //Appel de la fonction qui gère OTA
}