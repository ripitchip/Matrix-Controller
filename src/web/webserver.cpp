#include "webserver.h"
#include "../display/display.h"
#include "../storage/sdcard.h"
#include <FS.h>
#include <SD.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>

String generateFileListHTML()
{
    String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<title>ESP32 Matrix Image Selector</title>";
    html += "<style>"
            "body { font-family: Arial; text-align:center; background:#222; color:#fff; }"
            "h1 { color:#0f0; }"
            "ul { list-style:none; padding:0; display:flex; flex-wrap: wrap; justify-content: center; }"
            "li { margin: 10px; background:#333; padding:10px; border-radius:8px; }"
            "img { display:block; margin: 0 auto; border:1px solid #555; }"
            "button { background:#0f0; border:none; padding:5px 15px; cursor:pointer; border-radius:4px; margin-top:5px; }"
            "</style></head><body>";

    html += "<h1>ESP32 Matrix Image Selector</h1>";

    html += "<ul>";
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file)
    {
        String name = file.name();
        if (name.endsWith(".png"))
        {
            String rawName = name;
            rawName.replace(".png", ".raw");

            html += "<li>";
            html += "<img src='/images?file=" + name + "' width='64' height='64'>";

            html += "<form method='POST' action='/display'>";
            html += "<input type='hidden' name='file' value='" + rawName + "'>";
            html += "<button type='submit'>Display on Matrix</button>";
            html += "</form>";

            html += "</li>";
        }
        file = root.openNextFile();
    }
    html += "</ul></body></html>";
    return html;
}

void initWebServer(AsyncWebServer &server)
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", generateFileListHTML()); });

    server.on("/display", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        if (request->hasParam("file", true)) { 
            String fileName = request->getParam("file", true)->value();
            if (!fileName.startsWith("/")) fileName = "/" + fileName;
            displayFrame(fileName.c_str());
        }
        request->redirect("/"); });

    server.on("/images", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (request->hasParam("file")) {
            String fileName = request->getParam("file")->value();
            if (!fileName.startsWith("/")) fileName = "/" + fileName;
            request->send(SD, fileName, "image/png");
        } else {
            request->send(400, "text/plain", "No file specified");
        } });

    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
              { request->send(200, "text/plain", "Files uploaded successfully"); }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
              {
            static File uploadFile;

            if (index == 0) {
                Serial.printf("Upload start: %s\n", filename.c_str());
                String path = "/" + filename;
                uploadFile = SD.open(path, FILE_WRITE);
                if (!uploadFile) {
                    Serial.println("Failed to open file for writing");
                    return;
                }
            }

            if (uploadFile) uploadFile.write(data, len);

            if (final && uploadFile) {
                uploadFile.close();
                Serial.printf("Upload complete: %s (%u bytes)\n", filename.c_str(), (unsigned int)(index + len));
            } });

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    Serial.println("Routes installed: /, /display, /images, /upload");
    server.begin();
    Serial.println("✅ Web server started");
}
