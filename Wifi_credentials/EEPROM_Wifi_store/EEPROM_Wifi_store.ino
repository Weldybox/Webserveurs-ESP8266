    #include <ESP8266WiFi.h>
    #include <EEPROM.h>
     
    // Structure
    struct EEconf
    {
      char ssid[32];
      char password[64];
      char myhostname[32];
    };
     
    void setup()
    {
      // Initialisation des paramètres de connexion
      EEconf myconf = {              // A remplacer par :
                       "monSSID",    // le SSID du réseau,
                       "motdepasse", // le mot de passe du réseau
                       "nomhote"};   // le nom à donner au client MQTT
     
      // Variable pour la relecture
      EEconf readconf;
     
      Serial.begin(115200);
     
      // Initialisation EEPROM
      EEPROM.begin(sizeof(myconf));
      // Enregistrement
      EEPROM.put(0,myconf);
      EEPROM.commit();
     
      // Relecture et affichage
      EEPROM.get(0,readconf);
      Serial.println("\n\n\n");
      Serial.println(readconf.ssid);
      Serial.println(readconf.password);
      Serial.println(readconf.myhostname);
    }
     
    void loop()
    {
     
    }
