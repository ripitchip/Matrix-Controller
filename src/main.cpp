#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <FastLED.h>
#include <Adafruit_GFX.h>
#include <WiFi.h>
#include <secrets.h>

//----------------------------------------Panel configuration
#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1
#define PIN_E 32
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
#define CH_E PIN_E
#define CLK 16
#define LAT 4
#define OE 15
//----------------------------------------

//----------------------------------------SD Card SPI pins
#define SD_CS 32
#define SD_MOSI 21
#define SD_MISO 22
#define SD_SCK 18
//----------------------------------------

#define FRAME_COUNT 40  // Adjust based on your GIF
#define FRAME_DELAY 100 // ms per frame

MatrixPanel_I2S_DMA *dma_display = nullptr;
WiFiServer server(80);

//----------------------------------------Recursive SD tree print
void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listing directory: %s\n", dirname);
  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels)
        listDir(fs, file.name(), levels - 1);
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

//----------------------------------------Display one frame from SD
void displayFrame(const char *filename)
{
  File f = SD.open(filename, "r");
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
      if (f.available())
        dma_display->drawPixel(x, y, dma_display->color565(r, g, b));
      else
        break;
    }
  }

  f.close();
}

//----------------------------------------Setup
void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- ESP32 HUB75 + SD Card ---");

  // Mount SD card
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS, SPI, 25000000))
  {
    Serial.println("❌ SD Card Mount Failed!");
    while (true)
      delay(1000);
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE)
  {
    Serial.println("❌ No SD card detected!");
    while (true)
      delay(1000);
  }

  Serial.print("✅ SD Card Type: ");
  if (cardType == CARD_MMC)
    Serial.println("MMC");
  else if (cardType == CARD_SD)
    Serial.println("SDSC");
  else if (cardType == CARD_SDHC)
    Serial.println("SDHC");
  else
    Serial.println("UNKNOWN");

  listDir(SD, "/", 2);

  // Map HUB75 pins
  HUB75_I2S_CFG::i2s_pins _pins = {
      R1, G1, B1,
      R2, G2, B2,
      CH_A, CH_B, CH_C, CH_D, CH_E,
      LAT, OE, CLK};

  // Panel configuration
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;
  mxconfig.clkphase = false;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_100G;

  // Initialize display
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(155);
  dma_display->clearScreen();

  Serial.println("✅ Matrix Display Initialized");

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n✅ WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

//----------------------------------------Main loop
void loop()
{
  for (int i = 0; i < FRAME_COUNT; i++)
  {
    String path = "/frame" + String(i) + ".raw";
    displayFrame(path.c_str());
    delay(FRAME_DELAY);
  }
}
