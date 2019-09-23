#include <Arduino.h>
#include <SmarTC_Settings_SPIFFS.h>
#include <SmarTC_WiFi.h>
#include <SmarTC_OTA.h>
#include <SmarTC_MQTT.h>

#include "ESP8266RoomLightSensor.h"

SmarTC_Settings_SPIFFS settings = SmarTC_Settings_SPIFFS();
SmarTC_WiFi wifi = SmarTC_WiFi();
SmarTC_OTA ota = SmarTC_OTA();
SmarTC_MQTT mqtt = SmarTC_MQTT();

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  /*  _________                      .__  __  .__       .__
      \_   ___ \  ____   ____   ____ |__|/  |_|__| ____ |__|____    ____
      /    \  \/ /  _ \ / ___\ /    \|  \   __\  |/ ___\|  \__  \  /    \ 
      \     \___(  <_> ) /_/  >   |  \  ||  | |  \  \___|  |/ __ \|   |  \
       \______  /\____/\___  /|___|  /__||__| |__|\___  >__(____  /___|  /
              \/      /_____/      \/                 \/        \/     \/
  */

  Serial.println();
  Serial.println(F("********************************************************************"));
  Serial.println(F("_________                      .__  __  .__       .__               "));
  Serial.println(F("\\_   ___ \\  ____   ____   ____ |__|/  |_|__| ____ |__|____    ____  "));
  Serial.println(F("/    \\  \\/ /  _ \\ / ___\\ /    \\|  \\   __\\  |/ ___\\|  \\__  \\  /    \\ "));
  Serial.println(F("\\     \\___(  <_> ) /_/  >   |  \\  ||  | |  \\  \\___|  |/ __ \\|   |  \\"));
  Serial.println(F(" \\______  /\\____/\\___  /|___|  /__||__| |__|\\___  >__(____  /___|  /"));
  Serial.println(F("        \\/      /_____/      \\/                 \\/        \\/     \\/ "));
  Serial.println(F("********************************************************************"));
  Serial.println("SmarTC Room Light Sensor - ESP8266");
  Serial.println(F(__DATE__ " " __TIME__));
  Serial.println(F("Version 1.0.0"));
  Serial.println(F("********************************************************************"));

  // Pin Setup
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); // Power on LED
  pinMode(PIR_PIN, INPUT);

  // Init settings
  if (!settings.init())
    Serial.println("/!\\ Error initializing settings");
  else
    settings.printFilesList();

  // Init WiFi
  wifi.begin(settings.wifiSsid(),
             settings.wifiPwd(),
             settings.boardName());

  wifi.printMacAddress();

  if (!wifi.connect())
  {
    Serial.println(F("WiFi Connection Failed! Rebooting..."));
    delay(2000);
    ESP.restart();
  }

  Serial.println(F("WiFi Connected !"));
  wifi.wifiStatusInfo();

  // OTA Init
  ota.init(settings.otaPort(),
           settings.boardName(),
           settings.otaPwd());

  ota.start();

  // MQTT Init
  mqtt.init(settings.mqttUser(),
            settings.mqttPwd(),
            settings.mqttSrv(),
            settings.mqttPort(),
            settings.boardName());

  if (!mqtt.start())
    Serial.println(F("MQTT Connection Failed! Continue and try to connect on loop"));
}

void loop()
{
  if (!wifi.loop())
  {
    Serial.println(F("WiFi Connection lost! Rebooting..."));
    delay(1000);
    ESP.restart();
  }

  if (!ota.loop())
    Serial.println(F("OTA Process looping failed! Retry next loop ..."));

  if (!mqtt.loop())
    Serial.println(F("MQTT Connection lost! Retry next loop ..."));
}