#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FS.h>
#include <FastLED.h> // Needed for color conversion
#include <LittleFS.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <secrets.h>

//----------------------------------------Panel configuration
#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1
#define PIN_E 32 // Optional, depends on your panel
//----------------------------------------

//----------------------------------------HUB75E pins mapping
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
#define CH_E PIN_E // 32
#define CLK 16
#define LAT 4
#define OE 15
//----------------------------------------
#define FRAME_COUNT 40  // Adjust based on your GIF
#define FRAME_DELAY 100 // ms per frame

MatrixPanel_I2S_DMA *dma_display = nullptr;
uint16_t myRED, myGREEN, myBLUE, myWHITE, myBLACK;
WiFiServer server(80);
String header;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Mount LittleFS
  if (!LittleFS.begin(true))
  { // true = format if mount fails
    Serial.println("LittleFS mount failed!");
    while (true)
      delay(1000);
  }
  Serial.println("LittleFS mounted successfully.");

  // Map pins
  HUB75_I2S_CFG::i2s_pins _pins = {
      R1, G1, B1,
      R2, G2, B2,
      CH_A, CH_B, CH_C, CH_D, CH_E,
      LAT, OE, CLK};

  // Panel configuration
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  // mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_20M; // I2S speed
  mxconfig.driver = HUB75_I2S_CFG::FM6126A; // Working driver
  mxconfig.clkphase = false;                // Working scan timing
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_100G;

  // Initialize display
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(155); // 0-255
  dma_display->clearScreen();

  Serial.println("Matrix Display Initialized");

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void displayFrame(const char *filename)
{
  File f = LittleFS.open(filename, "r");
  if (!f)
  {
    Serial.printf("Failed to open frame file: %s\n", filename);
    return;
  }

  for (int y = 0; y < PANEL_RES_Y; y++)
  {
    for (int x = 0; x < PANEL_RES_X; x++)
    {
      uint8_t r = f.read();
      uint8_t g = f.read();
      uint8_t b = f.read();
      dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
    }
  }

  f.close();
}

void loop()
{
  for (int i = 0; i < FRAME_COUNT; i++)
  {
    String path = "/frame" + String(i) + ".raw"; // Example: /frame0.raw
    displayFrame(path.c_str());
    delay(FRAME_DELAY);
  }
}
