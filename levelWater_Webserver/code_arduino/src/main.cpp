/*------------------------------------------------------------------------------
28/03/2019

Code réalisé par : Julfi
Principe : Activer une pompe à travers un site web qui se coupe quand l'eau atteint sa limite.
Cadre : News letter JEUDIMAIL 28/03/2019 & projet

------------------------------------------------------------------------------*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>


const char* host = "maker.ifttt.com";

uint8_t go = 0; //Variable qui determine en quel mode de fonctionnement mettre la pompe (0, démarrage 1, test du niveau d'eau)
uint8_t motor = D5;

uint8_t stillWater = 0; //Variable qui compte le nombre d'itérations d'ajout d'eau sans succès
bool NoWater = 0; //Variable qui met ou non la pompe en mode arrêt dans le cas ou il n'y a plus d'eau dans le bac
unsigned int sensibilite = 500; /* =============================SENSIBILITE DU CAPTEUR A DEFINIR=============================*/

int volumeTot = 0;

unsigned long otaMillis;
unsigned long previousMillis = 0; //previousMillis du loop()
unsigned long previousMillisAddWater = 0;
unsigned long previousMillisMl = 0;
unsigned long previousMillisPompe = 0; //previousMillis pour compter le temps entre chaque test de remplissage afin de déterminer si le bac est vide ou non.
ESP8266WebServer server; //Déclaration de l'objet pour la librairie ESP8266WebServer


const char* hostString = "WaterDispenser";
const char* ssid = "";
const char* password = "";



/*------------------------------------------------------------------------------
Page web qui sera va être enregistrer dans la mémoire flash de l'ESP8266
------------------------------------------------------------------------------*/
char page[] PROGMEM =  R"=====(
  <!doctype html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <link href="https://fonts.googleapis.com/css?family=Abel" rel="stylesheet">
  <title>Niveau d'eau</title>
</head>

<body style="background-color: #1f1f1f;font-family: 'Abel', sans-serif;">
  <div class="global" style="display: flex;flex-direction:column;justify-content: center; align-items: center;">
    <div class="titre" style="display:flex;">
      <h1 style="color: #3a8ad5;font-size: 42px;">Ajouter de l'eau ?</h1>
    </div>
    <div class="paragraphe" style="display:flex;flex-direction: row;align-items: center;justify-content: space-around;width: 300px;">
        <p style="display: flex;color:white; font-size: 24px;">Niveau d'eau : </p>
        <div id="demo"><div style="background:#21d92c;border-radius:50%;width:50px;height:50px;"></div></div>
    </div>
    <div class="bouton" style="">
      <button style="margin-top: 20px;border:none;color:white;border-radius:12px 12px 12px 12px; background-color:#3a8ad5;font-size: 27px; font-weight: bold;width: 150px;height: 40px;text-align: center;" onclick="pompeControl()">ON</button>
    </div>
  </div>

</body>

<script>
  setInterval(function(){
    pompeControl() //Vérification toute les 5 minutes que le niveau d'eau est correct
  },10000);

  function pompeControl() {
  //document.getElementById("demo").style.background = "red";
    [1,2].forEach(function(i) {
      var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("demo").innerHTML = this.responseText;
            document.getElementById("volume").innerHTML = this.responseText;
          }
        };
        xhttp.open("GET", "/eau", true);
        xhttp.send();
    });
  }

document.addEventListener('DOMContentLoadded', pompeControl, false);
</script>
</html>
)=====";

void Push(){
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  String url = "/trigger/no_water/with/key/{key}";
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

    // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

void confOTA(){
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(hostString);
  //ArduinoOTA.setPassword(otamdp);

  ArduinoOTA.onStart([](){
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
    Serial.println("MAJ OTA is coming!");
    otaMillis=millis();
  });

  ArduinoOTA.onEnd([]{
    Serial.print("MAJ terminé en : ");
    Serial.print((millis() - otaMillis)/1000);
    Serial.println(" secondes.");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total){
    Serial.printf("Progression : %u%%\r", (progress / (total/100)));
  });

  ArduinoOTA.onError([](ota_error_t error){
    switch (error)
    {
      case OTA_AUTH_ERROR:  Serial.println("OTA_AUTH_ERROR");
        break;
      case OTA_BEGIN_ERROR: Serial.println("OTA_BEGIN_ERROR");
        break;
      case OTA_CONNECT_ERROR: Serial.println("OTA_CONNECT_ERROR");
        break;
      case OTA_END_ERROR: Serial.println("OTA_END_ERROR");
        break;
      case OTA_RECEIVE_ERROR: Serial.println("OTA_RECEIVE_ERROR");
        break;
      default: Serial.println("Erreur inconnue");
    }
  });

  ArduinoOTA.begin();

}

void getVolume(){
  char buffer [10];
  const char* volume = itoa (volumeTot,buffer , 10);
  server.send(200, volume);
  Serial.println("send");

}

void eauDisti(float volume){
  unsigned int currentMillis = millis();
  if (currentMillis - previousMillisMl >= 10000){
    if ( NoWater == 1 ){
      volumeTot -= 20;
    }
    volumeTot += (volume/1000)*8.5;
    Serial.print(volumeTot);
    Serial.println(" mL");
  }
}

/*------------------------------------------------------------------------------
Fonction qui détermine si le bac contient bien de l'eau à distribuer avec les paramètres (tmpCheck, Try, sensi)
qui sont respectivements le temps maximal de pompage sans niveau effectué, le nombres d'essais avant blocage
de la pompe et la sensibilitée du detecteur de niveau d'eau
------------------------------------------------------------------------------*/

void lvlEauCheck(unsigned int tmpCheck, unsigned int Try, unsigned int sensi){
  unsigned long pompeMillis = millis();
  //Boucle qui vérifie que le niveau d'eau d'exède pas la limite fixé par le capteur de niveau d'eau
  while(NoWater == 0){
  //  unsigned long check = millis();
    if (analogRead(A0) >= sensi){
      break;
    }
    //Si au bout de tmpCheck secondes le niveau n'est pas fait...
    if (millis() - pompeMillis >= tmpCheck){
      //On ajoute 1 à stillWater qui représente le nombre d'essaies infructueux
      stillWater ++;
      Serial.print("Il manque de l'eau plus que ");
      Serial.print(Try - stillWater);
      Serial.println(" essaies avant coupure");

      break;
    }

    delay(500);
    Serial.println("attendez!");
  }
  float duree = millis()-pompeMillis;
  //eauDisti(duree);
//  Serial.println(duree);
  //Si le niveau d'eau a atteint sa limite...
  if(analogRead(A0) >= sensi){
    stillWater=0; //On réinitialise la variable permettant de savoir s'il y a plus d'eau dans le bac
  }

  //Si le nombre d'essaies infructueux atteint la limite 'Try'...
  if (stillWater == Try){
    NoWater = 1; //On bloque la pompe avec la variable NoWater
    Serial.println("Coupure du mécanisme jusqu'à ajout d'eau");
    Push();
    stillWater = 0;

  }
  eauDisti(duree);
}

/*------------------------------------------------------------------------------
Fonction coupe la pompe selon certains paramètres
------------------------------------------------------------------------------*/
void coupePompe(){

  lvlEauCheck(5000,4, sensibilite); //Appel de fonction avec paramètres

  digitalWrite(motor, LOW);
  server.send(200, "text/plain", "<div style=\"background:#21d92c;border-radius:50%;width:50px;height:50px;\">"); //Si le niveau d'eau a atteint son maximum alors nous pouvons afficher une pastille verte
}

/*------------------------------------------------------------------------------
Fonction qui vérifie l'état du niveau d'eau, les variables de blocage et transmet
les informations en conséquence au serveur pour les afficher.
------------------------------------------------------------------------------*/
void pompeEau()
{
  if (go == 0 && analogRead(A0) <= sensibilite && NoWater == 0){
      server.send(200, "text/plain", "<div style=\"background:red;border-radius:50%;width:50px;height:50px;\"><p style=\"font-size:24px;color:white;padding-left:70px;\">Remplissage...</p>");
      digitalWrite(motor, HIGH);
      Serial.println("Moteur allumé");
      go++;
  }else if(go == 1 && NoWater == 0){
    coupePompe();
    go=0;
  }else if(go == 0 && analogRead(A0) >= sensibilite){
    coupePompe();
    go=0;
  }
  else if(NoWater==1){
    Serial.println("La pompe est off pour le moment on attend de l'eau");
    Serial.println("Moteur éteint");
    server.send(200, "text/plain", "<div style=\"background:red;border-radius:50%;width:50px;height:50px;\"><p style=\"font-size:24px;color:white;padding-left:70px;\">Plus d'eau...</p>");
    go=0;
  }


}

void setup()
{
  pinMode(A0,INPUT);
  pinMode(motor,OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  Serial.begin(115200);
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  confOTA();

  Serial.println("");
  Serial.print("IP Address: ");

  Serial.println(WiFi.localIP());


  /*IFTTTWebhook hook(IFTTT_API_KEY, IFTTT_EVENT_NAME);
  hook.trigger();*/
  /*------------------------------------------------------------------------------
  Selon le chemin des requêtes reçus on effectue soit l'appel de la page simplement, ou l'appel de la fonction 'pompeEau'
  ------------------------------------------------------------------------------*/
  server.on("/",[](){server.send_P(200,"text/html",page);}); //Fonction ajouté à la méthode 'server.on'
  server.begin();
  server.on("/eau",pompeEau);
  server.on("/volume",getVolume);

}

/*------------------------------------------------------------------------------
Selon le chemin des requêtes reçus on effectue soit l'appel de la page simplement, ou l'appel de la fonction 'ajoutEau'
------------------------------------------------------------------------------*/
void ajoutEau(unsigned int tmpCheck){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisAddWater >= tmpCheck){
    if (NoWater == 1 && analogRead(A0) >= 150){
      NoWater = 0;
      Serial.println("De l'eau a été ajouté 'NoWater = False;'");
      //iftttButtonPressedFlag = true;
      //triggerIftttEvent();
    }
    delay(10);
    previousMillis = millis();
    }

}
void loop()
{
  ajoutEau(5000);
  server.handleClient();
  /*unsigned long now = millis();
  if (now - previousMillis >= 1000){
    previousMillis = millis();
  }*/


  ArduinoOTA.handle();
}
