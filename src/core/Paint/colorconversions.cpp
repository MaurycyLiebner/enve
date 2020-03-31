// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "colorconversions.h"


// used mainly for loading layers (transparent PNG)
void rgba8_to_rgba16(uint8_t* src,
                     const int srcWidth,
                     uint16_t* dst,
                     const int dstWidth,
                     const int height) {
    for(int i = 0; i < height; i++) {
        uint8_t *srcLine = src + i * srcWidth * 4;
        uint16_t *dstLine = dst + i * dstWidth * 4;
        for(int j = 0; j < dstWidth; j++) {
            uint32_t r, g, b, a;
            r = *srcLine++;
            g = *srcLine++;
            b = *srcLine++;
            a = *srcLine++;

            // convert to fixed point (with rounding)
            r = (r * (1<<15) + 255/2) / 255;
            g = (g * (1<<15) + 255/2) / 255;
            b = (b * (1<<15) + 255/2) / 255;
            a = (a * (1<<15) + 255/2) / 255;

            // premultiply alpha (with rounding), save back
            *dstLine++ = (r * a + (1<<15)/2) / (1<<15);
            *dstLine++ = (g * a + (1<<15)/2) / (1<<15);
            *dstLine++ = (b * a + (1<<15)/2) / (1<<15);
            *dstLine++ = a;
        }
    }
}

// Naive conversion code from the internal MyPaint format and 8 bit RGB
void rgba16_to_rgba8_unpremultiplied(
        uint16_t* src,
        const int srcWidth,
        uint8_t* dst,
        const int dstWidth,
        const int height) {
    for(int i = 0; i < height; i++) {
        uint16_t *srcLine = src + i * srcWidth * 4;
        uint8_t *dstLine = dst + i * dstWidth * 4;
        for(int j = 0; j < srcWidth; j++) {
            uint32_t r, g, b, a;

            r = *srcLine++;
            g = *srcLine++;
            b = *srcLine++;
            a = *srcLine++;

            // un-premultiply alpha (with rounding)
            if(a != 0) {
//                r = ((r << 15)) / a;
//                g = ((g << 15)) / a;
//                b = ((b << 15)) / a;
                r = ((r << 15) + a/2) / a;
                g = ((g << 15) + a/2) / a;
                b = ((b << 15) + a/2) / a;
            } else {
                r = g = b = 0;
            }

            // Variant A) rounding
            const uint32_t add_r = (1<<15)/2;
            const uint32_t add_g = (1<<15)/2;
            const uint32_t add_b = (1<<15)/2;
            const uint32_t add_a = (1<<15)/2;
            *dstLine++ = (r * 255 + add_r) / (1<<15);
            *dstLine++ = (g * 255 + add_g) / (1<<15);
            *dstLine++ = (b * 255 + add_b) / (1<<15);
            *dstLine++ = (a * 255 + add_a) / (1<<15);
//            *dstLine++ = (r * 255) / (1<<15);
//            *dstLine++ = (g * 255) / (1<<15);
//            *dstLine++ = (b * 255) / (1<<15);
//            *dstLine++ = (a * 255) / (1<<15);
        }
    }
}

void rgba16_to_rgba8_premultiplied(
        uint16_t* src,
        const int srcWidth,
        uint8_t* dst,
        const int dstWidth,
        const int height) {
    for(int i = 0; i < height; i++) {
        uint16_t *srcLine = src + i * srcWidth * 4;
        uint8_t *dstLine = dst + i * dstWidth * 4;
        for(int j = 0; j < srcWidth; j++) {
            uint32_t r, g, b, a;

            r = *srcLine++;
            g = *srcLine++;
            b = *srcLine++;
            a = *srcLine++;

            const uint32_t add_r = (1<<15)/2;
            const uint32_t add_g = (1<<15)/2;
            const uint32_t add_b = (1<<15)/2;
            const uint32_t add_a = (1<<15)/2;
            *dstLine++ = (r * 255 + add_r) / (1<<15);
            *dstLine++ = (g * 255 + add_g) / (1<<15);
            *dstLine++ = (b * 255 + add_b) / (1<<15);
            *dstLine++ = (a * 255 + add_a) / (1<<15);
        }
    }
}

