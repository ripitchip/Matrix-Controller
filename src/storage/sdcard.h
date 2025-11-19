#pragma once
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "../config/pins.h"

bool initSD();
File openFile(const char *path, const char *mode = "r");