#pragma once
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <FS.h>
#include "../config/pins.h"

extern MatrixPanel_I2S_DMA *dma_display;

void initDisplay();
void displayFrame(const char *filename);
