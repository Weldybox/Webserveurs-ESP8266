
#include <Arduino.h>
/** Chargement du SSId et du MDP dans la mémoire de l'ESP */
void loadCredentials() {
  EEPROM.begin(512);
  EEPROM.get(0, ssid);
  EEPROM.get(0 + sizeof(ssid), password);
  char ok[2 + 1];
  EEPROM.get(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    ssid[0] = 0;
    password[0] = 0;
  }
  Serial.println("Récupération du SSID et du MDP stockés:");
  Serial.println(ssid);
  Serial.println(strlen(password) > 0 ? "********" : "<pas de mot de passe...>");
}

/** Sauvergarder le SSID et le MDP dans la mémoire de l'ESP  */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0 + sizeof(ssid), password);
  char ok[2 + 1] = "OK";
  EEPROM.put(0 + sizeof(ssid) + sizeof(password), ok);
  EEPROM.commit();
  EEPROM.end();
}