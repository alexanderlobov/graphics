#pragma once

#include <vector>

bool saveRGBtoJPEG(
        unsigned char *data,
        int width,
        int height,
        const char* fileName,
        int quality = 100);

bool readJPEGtoRGB(
        const char* const filename,
        std::vector<unsigned char>& rgbBuffer,
        int& width,
        int& height);
