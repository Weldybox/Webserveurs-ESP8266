#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
/*
   This example serves a "hello world" on a WLAN and a SoftAP at the same time.
   The SoftAP allow you to configure WLAN parameters at run time. They are not setup in the sketch but saved on EEPROM.
   Connect your computer or cell phone to wifi network ESP_ap with password 12345678. A popup may appear and it allow you to go to WLAN config. If it does not then navigate to http://192.168.4.1/wifi and config it there.
   Then wait for the module to connect to your wifi and take note of the WLAN IP it got. Then you can disconnect from ESP_ap and return to your regular WLAN.
   Now the ESP8266 is in your network. You can reach it through http://192.168.x.x/ (the IP you took note of) or maybe at http://esp8266.local too.
   This is a captive portal because through the softAP it will redirect any http request to http://192.168.4.1/
*/

/* Configuration du soft AP */
#ifndef APSSID
#define APSSID "ESP_ap_Température"
#define APPSK  "12345678"
#endif

const char *softAP_ssid = APSSID;
const char *softAP_password = APPSK;

/* Configuration du non d'hôte Ex: http://WeldyTemp.local */
const char *myHostname = "WeldyTemp";

/* Configuration de l'SSID et du mot de passe WIFI au démarrage (à condition d'avoir ) */
char ssid[32] = "";
char password[32] = "";

// Serveur DNS
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Serveur Web
ESP8266WebServer server(80);

/* Définition des paramètres IP du softAP */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);


/** Should I connect to WLAN asap? */
boolean connect;

/* Dernière fois que j'ai essayé de me connecter au WLAN */
unsigned long lastConnectTry = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

#include "tools.h"
#include "credentials.h"
#include "handleHttp.h"

void setup() {

  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuration du point d'accès...");
  //Configuration du softAP selon les paramètres que l'on précédemment définis
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(softAP_ssid, softAP_password);

  delay(500);

  Serial.print("L'adresse IP de l'AP est : ");
  Serial.println(WiFi.softAPIP());

  /* Configuration du serveur DNS, toutes les requêtes seront redirigées vers le soft AP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  /* Configuration des différentes pages accessibles */
  server.on("/", handleRoot);
  server.on("/wifi", handleWifi); 
  server.on("/wifisave", handleWifiSave);
  server.on("/generate_204", handleRoot);  //Page du portail captif android.
  server.on("/fwlink", handleRoot);  //page du portail captif Windows.
  server.onNotFound(handleNotFound);
  server.begin(); // Lacement du serveur Web
  Serial.println("HTTP server started");
  loadCredentials(); // Tentative de chargement des données WIFI sauvegarder dans l'ESP
  connect = strlen(ssid) > 0; // On essaye ensuite de se connecté
}

//Tentative de connexion au WIFI
void connectWifi() {
  Serial.println("Connection en tant que client WIFI...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  int connRes = WiFi.waitForConnectResult();

  //Résultat de la tentative de connexion au WIFI
  Serial.print("Reponse : ");
  Serial.println(connRes);
}

void loop() {
  //Si connect est true ça veut dire qu'un SSID et un mdp on été sauvegardés.
  if (connect) {
    Serial.println("Requête de connexion");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  {
    unsigned int s = WiFi.status();

    //Si nous n'avons pas réussi à se connecter
    if (s == 0 && millis() > (lastConnectTry + 60000)) {
      /* On attend 1 minutes avec de retater une connexion */
      connect = true;
    }

    if (status != s) { // Si le status du WIFI change
      Serial.print("Status: ");
      Serial.println(s);
      status = s;
      if (s == WL_CONNECTED) {
        /* Si celui-ci est connecté */

        /* On affiche sur le port série l'adresse IP de l'ESP et le nom du réseau sur lequel on est connecté */
        Serial.println("");
        Serial.print("Connected to ");
        Serial.println(ssid);
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        // vérificationd de la disponbilité du nom de domaine
        if (!MDNS.begin(myHostname)) {
          Serial.println("Error setting up MDNS responder!");
        } else {
          Serial.println("mDNS responder started");
          // Add service to MDNS-SD
          MDNS.addService("http", "tcp", 80);
        }
      } else if (s == WL_NO_SSID_AVAIL) {
        WiFi.disconnect();
      }
    }
    if (s == WL_CONNECTED) {
      MDNS.update();
    }
  }

  dnsServer.processNextRequest();
  server.handleClient();
}

