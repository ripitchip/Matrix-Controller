#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <secrets.h>

#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1
#define PIN_E 32

#define R1 25
#define G1 26
#define B1 27
#define R2 14
#define G2 12
#define B2 13
#define CH_A 23
#define CH_B 19
#define CH_C 5
#define CH_D 17
#define CH_E PIN_E
#define CLK 16
#define LAT 4
#define OE 15

#define SD_CS 32
#define SD_MOSI 21
#define SD_MISO 22
#define SD_SCK 18

MatrixPanel_I2S_DMA *dma_display = nullptr;
AsyncWebServer server(80);

void displayFrame(const char *filename)
{
  File f = SD.open(filename, "r");
  if (!f)
  {
    Serial.printf("Failed to open frame file: %s\n", filename);
    return;
  }

  dma_display->clearScreen();

  for (int y = 0; y < PANEL_RES_Y; y++)
  {
    for (int x = 0; x < PANEL_RES_X; x++)
    {
      if (!f.available())
        break;
      uint8_t r = f.read();
      uint8_t g = f.read();
      uint8_t b = f.read();
      dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
    }
  }
  f.close();
}

String generateFileListHTML()
{
  String html = "<html><body><h1>ESP32 Matrix Image Selector</h1><ul>";
  File root = SD.open("/");
  File file = root.openNextFile();
  while (file)
  {
    String name = file.name();
    if (name.endsWith(".png"))
    {
      html += "<li>";
      html += "<img src='/images?file=" + name + "' width='64' height='64'> ";
      String rawName = name;
      rawName.replace(".png", ".raw");
      html += "<a href='/display?file=" + rawName + "'>Display on Matrix</a></li>";
    }
    file = root.openNextFile();
  }
  html += "</ul></body></html>";
  return html;
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- ESP32 HUB75 + SD Card ---");

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS, SPI, 25000000))
  {
    Serial.println("❌ SD Card Mount Failed!");
    while (true)
      delay(1000);
  }

  if (SD.cardType() == CARD_NONE)
  {
    Serial.println("❌ No SD card detected!");
    while (true)
      delay(1000);
  }

  HUB75_I2S_CFG::i2s_pins _pins = {R1, G1, B1, R2, G2, B2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;
  mxconfig.clkphase = false;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_8M;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(155);
  dma_display->clearScreen();

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

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", generateFileListHTML()); });

  server.on("/display", HTTP_GET, [](AsyncWebServerRequest *request)
            {
                if (request->hasParam("file"))
                {
                    String fileName = request->getParam("file")->value();
                    if (!fileName.startsWith("/")) fileName = "/" + fileName;
                    displayFrame(fileName.c_str());
                    request->send(200, "text/plain", "Displaying: " + fileName);
                }
                else
                {
                    request->send(400, "text/plain", "No file specified");
                } });

  server.on("/images", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("file"))
    {
        String fileName = request->getParam("file")->value();
        if (!fileName.startsWith("/")) fileName = "/" + fileName;
        request->send(SD, fileName, "image/png"); 
    }
    else
    {
        request->send(400, "text/plain", "No file specified");
    } });

  server.begin();
  Serial.println("✅ Web server started");
}

void loop()
{
}
