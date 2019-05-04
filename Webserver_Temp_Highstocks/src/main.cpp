#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <FS.h>
#include <ESP8266mDNS.h>

#define MSECOND  1000
#define MMINUTE  60*MSECOND
#define MHOUR    60*MMINUTE
#define MDAY 24*MHOUR

unsigned long utcOffsetInSeconds = 0;


/*--------------------------------------------------------------------------------
Fonction qui permet de déterminer si l'on est en heure d'été ou d'hiver
--------------------------------------------------------------------------------*/
int EteOuHiver(unsigned int utcOffsetInSeconds, const char* ete){
  if(ete == "ete"){
    return utcOffsetInSeconds;
  }
}

/*--------------------------------------------------------------------------------
Définitions du client NTP pour récupérer les informations
--------------------------------------------------------------------------------*/
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org",utcOffsetInSeconds);


// Structure de donnée enregistrer dans la mémoire flash
struct EEconf {
  char ssid[32];
  char password[64];
  char myhostname[32];
};
EEconf readconf;


unsigned long previousLoopMillis = 0;
unsigned long otamillis; //Variable qui va permettre de calculer le temps d'éxecution d'OTA

Adafruit_BME280 bme; //Objet bme pour gérer la librairie BME
ESP8266WebServer server; //Définition de l'objet webserver



/*--------------------------------------------------------------------------------
Fonction de programmation OTA
--------------------------------------------------------------------------------*/
void confOTA() {
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(readconf.myhostname);

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

void addData() {
  char strbuffer[64];
  int timestamp = timeClient.getEpochTime();
  snprintf(strbuffer, 64, "%d,%.2f;",
    timestamp,
    bme.readTemperature());
    
  Serial.println(strbuffer);
  File f = SPIFFS.open("/temperature.csv", "a+");
  if (!f) {
    Serial.println("erreur ouverture fichier!");
  } else {
      f.print(strbuffer);
      f.close();
  }
}

void setup()
{

  Serial.begin(115200);
 


  // Initialisation EEPROM
  EEPROM.begin(sizeof(readconf));
  // Enregistrement

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


  if (!MDNS.begin("WeldyChart")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  EEPROM.get(0,readconf);
  Serial.println("\n\n\n");
  Serial.println(readconf.ssid);
  Serial.println(readconf.password);


  // Connexion au Wifi
  Serial.print(F("Connexion Wifi AP"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(readconf.ssid, readconf.password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("\r\nWiFi connecté"));
  Serial.print(F("Mon adresse IP: "));
  Serial.println(WiFi.localIP());

  //SPIFFS.begin();
  if(!SPIFFS.begin()) {
  Serial.println("Erreur initialisation SPIFFS");
}
  //Initialisation de la connexion avec le serveur NTP
 timeClient.begin();
  // configuration OTA
  confOTA();
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/index.html", SPIFFS, "/index.html");
  server.serveStatic("/temperature.csv", SPIFFS, "/temperature.csv");
  server.begin();

}

void loop()
{

  /*--------------------------------------------------------------------------------
  Boucle qui va envoyer toute les X secondes le timestamp et la températureu au
  webserveur
  --------------------------------------------------------------------------------*/
  unsigned long currentLoopMillis = millis();
  if(currentLoopMillis - previousLoopMillis >= 30*MSECOND){
    addData();

    FSInfo fs_info;
    SPIFFS.info(fs_info); 
    File f1 = SPIFFS.open("/temperature.csv","r");
    int data  = f1.size();
    f1.close();
    File f2 = SPIFFS.open("/index.html","r");
    int html  = f2.size();
    f2.close();
    int total = data + html;
    Serial.println(total);
    Serial.println(fs_info.totalBytes);

    if(total < fs_info.totalBytes){
      Serial.println("ok");
    }else{
      Serial.println("nok");
    }
    previousLoopMillis = millis();

}


  ArduinoOTA.handle(); //Appel de la fonction qui gère OTA
  server.handleClient(); //Appel de la fonctino qui gère la communication avec le serveur web
  timeClient.update(); //Récupération du timestamp en temps réel
}
