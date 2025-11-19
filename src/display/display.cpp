#include "display.h"
#include <Arduino.h>
#include <SD.h>

MatrixPanel_I2S_DMA *dma_display = nullptr;

void initDisplay()
{
    HUB75_I2S_CFG::i2s_pins _pins = {R1, G1, B1, R2, G2, B2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
    HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
    mxconfig.driver = HUB75_I2S_CFG::FM6126A;
    mxconfig.clkphase = false;
    mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_8M;
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->begin();
    dma_display->setBrightness8(155);
    dma_display->clearScreen();
}

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
