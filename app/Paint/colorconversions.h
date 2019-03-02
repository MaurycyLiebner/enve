#ifndef COLORCONVERSIONS_H
#define COLORCONVERSIONS_H
#include <stdint-gcc.h>

void rgba8_to_rgba16(uint8_t* src,
                     const int& srcWidth,
                     uint16_t* dst,
                     const int& dstWidth,
                     const int& height);
// Naive conversion code from the internal MyPaint format and 8 bit RGB
void rgba16_to_rgba8_unpremultiplied(uint16_t* src,
                    const int& srcWidth,
                    uint8_t* dst,
                    const int& dstWidth,
                    const int& height);


void rgba16_to_rgba8_premultiplied(
        uint16_t* src,
        const int& srcWidth,
        uint8_t* dst,
        const int& dstWidth,
        const int& height);
#endif // COLORCONVERSIONS_H
