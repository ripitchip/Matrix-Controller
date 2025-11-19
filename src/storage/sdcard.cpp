#include "sdcard.h"
#include <Arduino.h>

bool initSD()
{
    SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
    if (!SD.begin(SD_CS, SPI, 25000000))
    {
        Serial.println("❌ SD Card Mount Failed!");
        return false;
    }
    if (SD.cardType() == CARD_NONE)
    {
        Serial.println("❌ No SD card detected!");
        return false;
    }
    return true;
}

File openFile(const char *path, const char *mode)
{
    return SD.open(path, mode);
}
