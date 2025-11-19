#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "config/settings.h"
#include "display/display.h"
#include "web/webserver.h"
#include "storage/sdcard.h"

AsyncWebServer server(80);

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- ESP32 HUB75 + SD Card ---");

  if (!initSD())
    while (true)
      delay(1000);

  initDisplay();
  displayFrame("/untitled.raw");

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  initWebServer(server);
}

void loop()
{
  // nothing needed here; server and display run in background
}
