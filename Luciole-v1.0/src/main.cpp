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
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>

/*--------------------------------------------------------------------------------
Variables qui peremttent de manipuler des constantes de temps
--------------------------------------------------------------------------------*/
#define MSECOND  1000
#define MMINUTE  60*MSECOND
#define MHOUR    60*MMINUTE
#define MDAY 24*MHOUR

/*--------------------------------------------------------------------------------
Variable qui stock les numéros de broche de chaque MOSFET.
--------------------------------------------------------------------------------*/
#define REDPIN 13
#define GREENPIN 12
#define BLUEPIN 14

int dataSmartEcl[3]; //Tableau qui va contenir les 3 temps unix nécessaire au traitement.


unsigned long utcOffsetInSeconds = 7200; 

/*--------------------------------------------------------------------------------
Variables qui définissent le mode de fonctionement et le taux de rafraichissement
--------------------------------------------------------------------------------*/
unsigned long refresh;

String mode = "Desative"; //Le mode de fonctionement smart des LEDs, de base désactivé

uint8_t go = 0; //Variable qui se devient vrai lorsque l'on doit supprimer une valeure en mémoire de l'ESP.
     

unsigned long previousLoopMillis = 0; //Variable qui permet d'actualiser la couleur des LEDs toute les X secondes en mode Smart Eclairage

/*--------------------------------------------------------------------------------
Définition des objets nécessaire dans la suite du programme.
--------------------------------------------------------------------------------*/
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org",utcOffsetInSeconds);  

HTTPClient http;

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
Fonction qui retourne si l'on est plus proche du levé ou coucher de soleuil.
--------------------------------------------------------------------------------*/
String sunPosition(int dataSmartEcl[], uint8_t longueur){
  
  //Compare la différence entre le timestamp et les valeurs de sunrise et sunset
  //Si la différence de seconde entre l'heure actuelle et l'heure de levé de soleil est supérieur à la différence pour le coucher de soleil
  if(abs(dataSmartEcl[2] - dataSmartEcl[0]) > abs(dataSmartEcl[2] - dataSmartEcl[1])){
    return "soir"; //On considère qu'on est dans la période "soir"
  }else{
    return "matin"; //Sinon on considère qu'on est dans la période matin
  }
}

/*--------------------------------------------------------------------------------
Fonction qui va retourner l'écart en seconde entre le jour et la nuit
--------------------------------------------------------------------------------*/
uint16_t checkTime(int dataSmartEcl[], uint8_t longueur){
  uint8_t index;
  //Selon si l'on est le matin ou le soir on définir l'index de comparaison 'sunset/sunrise'
  if(sunPosition(dataSmartEcl, 3) == "matin"){index = 0;}
  else{index=1;}

  //On vérifie que l'heure du jour est au alentour de l'heure de sunset ou sunrise
  int ecart = dataSmartEcl[2] - dataSmartEcl[index]; //L'écart c'est la différence entre l'heur actuelle et l'heure de coucher/levé de soleil
  Serial.println(ecart);
  if(ecart < 0 && ecart > -3600){ //Si c'ette écart se trouve dans la tranche de transition des LEDs

    int result = map(ecart, -3600, 0, 0, 3600); //On retrourne un résultat positif
    return result;
  }else if(ecart < -3600){return 0;} //Si l'écart est inférieur à la valeur de tranche basse on retourne 0
  else if(ecart > 0){return 3600;} //Sinon si l'écart est supérieur à la valeure de tranche haut on retourne 3600
  else{return 0;} //Sinon on retourne 0

}

/*--------------------------------------------------------------------------------
Fonction qui retourne les valeur de rouge vert et bleu en fonction de l'écart
entre le jour et la nuit
--------------------------------------------------------------------------------*/
void displayColors(int dataSmartEcl[],uint8_t longueur){
  /*--------------------------------------------------------------------------------
  Selon l'a position du soleil nous cherchons à aller vers le blanc ou le orange/rouge
  --------------------------------------------------------------------------------*/
  Serial.println(sunPosition(dataSmartEcl, 3));
  if(sunPosition(dataSmartEcl, 3) == "soir"){ //S'il on se trouve dans la tranche soir
    Serial.println("soir");

    //On map les valeurs de vert et bleu de 255 (couleur blanche) au valeurs attendue en sortie.
    uint8_t rouge = 255;
    uint8_t vert = map(checkTime(dataSmartEcl, 3),0,3600,255,85);
    uint8_t bleu = map(checkTime(dataSmartEcl, 3),0,3600,255,0);
    //Puis on set le bandeau de LED tel quel.
    analogWrite(REDPIN, rouge);
    analogWrite(GREENPIN, vert);
    analogWrite(BLUEPIN, bleu);
    
    //On envoie également cette valeure sur le websocket pour avoir un retour du côté client.
    String rgb = ("rgb("+String(rouge, DEC) +","+String(vert,DEC)+","+String(bleu,DEC)+")");
    Serial.println(rgb);
    webSocket.sendTXT(0,rgb);

  }else{
    Serial.println("journee"); //Dans le cas contraire on est dans la seconde partie de tranche

    //On map les valeus de vert et bleu allant des valeurs de couleurs chaudes à 255 (couleurs blanche)
    uint8_t rouge = 255;
    uint8_t vert = map(checkTime(dataSmartEcl, 3),0,3600,85,255);
    uint8_t bleu = map(checkTime(dataSmartEcl, 3),0,3600,0,255);

    //Puis on set le bandeau de LED te quel.
    analogWrite(REDPIN, rouge);
    analogWrite(GREENPIN, vert);
    analogWrite(BLUEPIN, bleu);

    //ON envoie également cette valeure sur le websocket pour avoir un retour du côté client.
    String rgb = ("rgb("+String(rouge, DEC) +","+String(vert,DEC)+","+String(bleu,DEC)+")");
    Serial.println(rgb);
    webSocket.sendTXT(0,rgb);
  }

}

/*--------------------------------------------------------------------------------
Fonction qui retourne l'heure de sunset ou de sunrise (en unix timestamp)
--------------------------------------------------------------------------------*/
int requete(String Apikey, String ville,String type){
  String requeteHTTP =("http://api.openweathermap.org/data/2.5/weather?q=" + ville + "&APPID="
  + Apikey); 
  int timestamp;

  http.begin(requeteHTTP);  //Specify request destination
  int httpCode = http.GET();

  if (httpCode > 0) { //Check the returning code
 
  String payload = http.getString();   //Get the request response payload
  //Serial.println(payload);      //Print the response payload

  DynamicJsonDocument doc(765);
  DeserializationError error = deserializeJson(doc, payload);
  if(type == "sunrise"){
    timestamp = doc["sys"]["sunrise"];
  }else if(type == "sunset"){
    timestamp = doc["sys"]["sunset"];
  }
  

  }else{
    timestamp = 0;
  }
  http.end();   //Close connection
  return timestamp;
}

/*--------------------------------------------------------------------------------
Fonction qui suprrime la première ligne que plus de 4 couleurs ont été enregistré
--------------------------------------------------------------------------------*/
void suprdata(String data[55], int tabIndex[5]){
  File ftemp = SPIFFS.open("/save.csv", "w"); //On ouvre en mode écriture le fichier save.csv

  for(uint8_t x=1;x<5;x++){ //Pour chaques couleurs sauvegardé
    char msg[60];
    int index = tabIndex[x]; //On récupère la position de chaque élément
    String ligne = data[index]; //Puis on récupère la couleur voulue
    sprintf(msg, "%s;",ligne.c_str()); //On concatène le point virgule avec le code couleur

    ftemp.print(msg);
  } 

  ftemp.close();
}

/*--------------------------------------------------------------------------------
Test s'il faut suprrimer la première couleur entrée.
--------------------------------------------------------------------------------*/
bool checkSpace(uint8_t count){
  go += count;
  if(go >= 5){
    return 1;
  }else{
    return 0;
  }
}

/*--------------------------------------------------------------------------------
Supression de des couleurs enregistrer en trop.
--------------------------------------------------------------------------------*/
void suprSelect(){
  File file = SPIFFS.open("/save.csv", "r");
 
  //Tableau à sauvegarder dans le fichier temporaire
  String save[55] = {};
  int index[5] = {};
  int longueur=1;


  for(uint8_t i=0;i<5;i++){ //On répète la boucle 5 fois pour chaque couleur
    String part = file.readStringUntil(';'); //On lie la ligne jusqu'au changement de couleur
    if(longueur > 1){ //On ne prend pas en compte la première
      save[longueur] = part; //On ajoute au tableau save la couleur
      index[i] = longueur; //On ajoute au tableau index l'index de la couleur
    }

    longueur += part.length();
  }
  go --;
  file.close(); 

  suprdata(save, index);
}

/*--------------------------------------------------------------------------------
Fonction qui traite les requêtes websocket arrivant depuis le serveur web.
--------------------------------------------------------------------------------*/
void addData(uint16_t couleur, uint8_t * couleurSave){
  uint16_t save = (uint16_t) strtol((const char *) &couleurSave[2], NULL, 10);
  File f = SPIFFS.open("/save.csv", "a+");
  if (!f) {
    Serial.println("erreur ouverture fichier!");
  }else{
    if(couleur == 'R'){
        char buffred[16];
        sprintf(buffred,"%d,",save);
        Serial.println(buffred);
        f.print(buffred);
    }else if(couleur == 'G'){
        char buffgreen[16];
        sprintf(buffgreen,"%d,",save);
        Serial.println(buffgreen);
        f.print(buffgreen);
        
    }else if(couleur == 'B'){
        char buffblue[16];
        sprintf(buffblue,"%d;",save);
        Serial.println(buffblue);
        f.print(buffblue);

        //On regarde si le nombre de couleur sauvegardé n'exède pas 5
        if(checkSpace(1)){
          Serial.println("true");

          //Si c'est le cas on libère de l'espace dans la mémoire
          suprSelect();
        }
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

      Serial.print("rouge = ");
      Serial.println(couleur);
    }
    if(payload[0] == 'G'){
      analogWrite(GREENPIN, couleur);

      Serial.print("vert = ");
      Serial.println(couleur);
    }
    if(payload[0] == 'B'){

      Serial.print("bleu = ");
      Serial.println(couleur);
    }
    if(payload[0] =='s'){
      addData(payload[1],payload);
    }
    if(payload[0] == '#'){
      if(payload[1] == '1'){

        dataSmartEcl[0] = (requete("5258129e3a2c4e8144a8c755cfb8e97d","La rochelle","sunrise")+utcOffsetInSeconds);
        dataSmartEcl[1] = (requete("5258129e3a2c4e8144a8c755cfb8e97d","La rochelle","sunset")+utcOffsetInSeconds);
        dataSmartEcl[2] = (timeClient.getEpochTime());

        if (checkTime(dataSmartEcl, 3)==0){
          refresh = 10*MMINUTE;
          mode="Active";
          Serial.println("mode = active");
          displayColors(dataSmartEcl,3);
        }else if(checkTime(dataSmartEcl, 3)>=0){
          refresh = 5*MMINUTE;
          mode="Process";
          Serial.println("mode = process");
          displayColors(dataSmartEcl,3);
        }
      
      }else{
        mode="Desative";
        Serial.println("mode = desactive");
      }
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

  timeClient.begin();
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent); //Fonction de callback si l'on reçois un message du Webserveur

}

        
void loop() {

  /*--------------------------------------------------------------------------------
  Test iteratifs pour l'éclairage intelligent
  --------------------------------------------------------------------------------*/
  unsigned long currentLoopMillis = millis();
  if(currentLoopMillis - previousLoopMillis >= refresh){
    dataSmartEcl[2] = (timeClient.getEpochTime());
    if(mode=="Active" or mode=="Process"){
      displayColors(dataSmartEcl,3);
    }
    previousLoopMillis = millis();
  }
  /*--------------------------------------------------------------------------------
  Les watchdogs!
  --------------------------------------------------------------------------------*/
  timeClient.update();
  ArduinoOTA.handle(); //Appel de la fonction qui gère OTA
  webSocket.loop(); //Appel de la fonction qui gère la communication avec le websocket
  server.handleClient(); //Appel de la fonctino qui gère la communication avec le serveur web
}
