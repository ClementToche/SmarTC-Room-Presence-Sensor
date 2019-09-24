#include <Arduino.h>
#include <SmarTC_Settings.h>
#include <SmarTC_WiFi.h>
#include <SmarTC_OTA.h>
#include <SmarTC_MQTT.h>
#include <SmarTC_VEML6070.h>

#include "ESP8266RoomLightSensor.h"

SmarTC_Settings settings = SmarTC_Settings();
SmarTC_WiFi wifi = SmarTC_WiFi();
SmarTC_OTA ota = SmarTC_OTA();
SmarTC_MQTT mqtt = SmarTC_MQTT();
SmarTC_VEML6070 uvs = SmarTC_VEML6070(VEML6070_1_T, 270);

unsigned long last_sense = 0;

// TODO: Trace library
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
  Serial.println(F("Version " PROJECT_VERSION));
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
  wifi.init(settings.wifiSsid(),
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

  // VEML6070 Init
  if (!uvs.launch())
    Serial.println(F("VEML6070 launch failure!"));

  digitalWrite(LED_BUILTIN, HIGH);
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

  // PIR Detection Section
  if (digitalRead(PIR_PIN) == 1)
  {
    digitalWrite(LED_BUILTIN, LOW);

    if (millis() - last_sense > 10000)
    {
      last_sense = millis();
      Serial.println("Detect");
      mqtt.pirSense(); // Send PIR Status
      mqtt.uvSense(uvs.getUV()); // Send UV Status
    }
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Daily reboot
  if (millis() > 24 * 60 * 60 * 1000)
    ESP.reset();

  yield();
}