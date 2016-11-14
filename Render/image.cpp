#include "image.h"
#include <turbojpeg.h>
#include <fstream>
#include <iostream>

using namespace std;

bool saveRGBtoJPEG(
        unsigned char *data,
        int width,
        int height,
        const char* const fileName,
        int quality)
{
    tjhandle tj = tjInitCompress();
    unsigned long jpegSize = 0;
    unsigned char* jpegBuffer = 0;
    int result = tjCompress2(
            tj, data, width, 3*width, height, TJPF_RGB,
            &jpegBuffer, &jpegSize, TJSAMP_444, quality,
            TJFLAG_BOTTOMUP);
    bool writeResult = false;
    if (0 == result) {
        ofstream f(fileName, ios::out | ios::binary);
        f.write((const char*)jpegBuffer, jpegSize);
        writeResult = f;
        f.close();
    }

    tjFree(jpegBuffer);
    tjDestroy(tj);

    return 0 == result && writeResult;
}

bool readJPEGtoRGB(
        const char* const filename,
        std::vector<unsigned char>& rgbBuffer,
        int& width,
        int& height)
{
    tjhandle tj = tjInitDecompress();

    ifstream f(filename, ios::in | ios::binary);
    if (!f) {
        std::cerr << "Can't open file " << filename << '\n';
        return false;
    }

    f.seekg(0, f.end);
    int length = f.tellg();
    f.seekg(0, f.beg);

    vector<unsigned char> jpegBuffer(length);
    f.read((char*)&jpegBuffer[0], length);

    if (!f) {
        std::cerr << "Can't read file " << filename << '\n';
        return false;
    } else {
        std::cerr << "File " << filename << " successfully read\n";
    }

    int jpegSubsamp;

    if(0 != tjDecompressHeader2(tj, jpegBuffer.data(), length, &width, &height,
            &jpegSubsamp))
    {
        return false;
    }

    rgbBuffer.resize(width * height * 3);

    if (0 != tjDecompress2(tj,
        jpegBuffer.data(), length, rgbBuffer.data(),
        width, 3 * width, height, TJPF_RGB, /*TJFLAG_BOTTOMUP*/ 0))
    {
        return false;
    }

    tjDestroy(tj);

    return true;
}
