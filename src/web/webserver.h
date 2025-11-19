#pragma once
#include <ESPAsyncWebServer.h>

void initWebServer(AsyncWebServer &server);
String generateFileListHTML();
