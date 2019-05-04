#include <Arduino.h>


/** Redirige vers me portail captif dans le cas nous recevons une requête vers un autre domaine. Retourne vrai de ce cas. */
boolean captivePortal() {

  //Si le serveur n'a pas d'adresse IP et que le nom de domaine de mon ESP n'est pas égale à "myHostname.local"
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    Serial.println("Requête redirigé vers un portail captif");
    //On redirige la requête vers l'adrresse local.
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // La requête 302 permet de faire une redirection non permanente.
    server.client().stop(); // On ne précise pas la longueur du contenue donc on stop manuellement le serveur
    return true;
  }
  return false;
}


/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  Serial.println("wifi save");
  //
  server.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
  server.arg("p").toCharArray(password, sizeof(password) - 1);
  server.sendHeader("Location", "wifi", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = strlen(ssid) > 0; // Request WLAN connect with new credentials if there is a SSID
}

void handleNotFound() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += server.uri();
  message += F("\nMethod: ");
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += server.args();
  message += F("\n");

  for (uint8_t i = 0; i < server.args(); i++) {
    message += String(F(" ")) + server.argName(i) + F(": ") + server.arg(i) + F("\n");
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);
}

/** Page de configuration du Wifi */
void handleWifi() {
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<html><head></head><body>"
            "<h1>Configuration Wifi</h1>");
  //Nous indique sur qu'elle softAP on est connecté
  if (server.client().localIP() == apIP) {
    Page += String(F("<p>Vous êtes connecté au softAP : ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>Vous êtes connecté au réseau : ")) + ssid + F("</p>");
  }
  //Nous indique la configuration actuelle du softAP et du WLAN si l'on est connecté
  Page +=
    String(F(
             "\r\n<br />"
             "<table><tr><th align='left'>SoftAP config</th></tr>"
             "<tr><td>SSID ")) +
    String(softAP_ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>Configuration actuelle WLAN</th></tr>"
      "<tr><td>SSID ") +
    String(ssid) +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br/>"
      "<table><tr><th align='left'>Liste de point d'accès </th></tr>");
  Serial.println("On commence le scan");
  int n = WiFi.scanNetworks(); //Scna les réseaux disponibles à proximité
  Serial.println("Scan effectué");
  //Si le nombre de réseau trouvé est supérieur à 0
  if (n > 0) {
    //Pour chaques on va afficher le SSID et la puissance
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    //Sinon on indique qu'il n'y a aucun réseau à proximité
    Page += F("<tr><td>Pas de WLAN trouvé</td></tr>");
  }

  //On affiche en-dessous des champs pour entrer le SSID et le mot de passe du WIFI sur lequel on veut se connecter.
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Se connecter:</h4>"
            "<input type='text' placeholder='Nom du réseau' name='n'/>"
            "<br /><input type='password' placeholder='Mot de passe' name='p'/>"
            "<br /><input type='submit' value='Se connceter/disconnecter'/></form>"
            "<p>You may want to <a href='/'>Revenir à la page d'accueil</a>.</p>"
            "</body></html>");
  server.send(200, "text/html", Page);
  server.client().stop(); // On stop ensuite le client puisque nous n'avons pas définis de longueur de contenue.
}

/* Page de garde du serveur web et du portail captif */
void handleRoot() {
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
   return;
  }
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");

  //On affiche la page d'accueil
  String Page;
  Page += F(
            "<html><head></head><body>"
            "<h1>HELLO WORLD!!</h1>");
  if (server.client().localIP() == apIP) {//On indique à l'utilisatreur sur quel point d'accès il est connecté
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  Page += F(//On lui demande ensuite s'il veut configurer l'accès Wifi.
            "<p>Retourné à la page d'accueil <a href='/wifi'>config the wifi connection</a>.</p>"
            "</body></html>");

  server.send(200, "text/html", Page);
}
