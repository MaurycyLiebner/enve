/*
 * Copyright (c) 2005 Dmitry Baryshev <ksquirrel.iv@gmail.com>
 */

/* This file is part of the KDE libraries
    Copyright (C) 1998, 1999, 2001, 2002 Daniel M. Duley <mosfet@kde.org>
    (C) 1998, 1999 Christian Tibirna <ctibirna@total.net>
    (C) 1998, 1999 Dirk A. Mueller <mueller@kde.org>
    (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
    (C) 2000 Josef Weidendorfer <weidendo@in.tum.de>
    (C) 2004 Zack Rusin <zack@kde.org>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

//
// ===================================================================
// Effects originally ported from ImageMagick for PixiePlus, plus a few
// new ones. (mosfet 05/26/2003)
// ===================================================================
//
/*
 Portions of this software are based on ImageMagick. Such portions are clearly
marked as being ported from ImageMagick. ImageMagick is copyrighted under the
following conditions:

Copyright (C) 2003 ImageMagick Studio, a non-profit organization dedicated to
making software imaging solutions freely available.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files ("ImageMagick"), to deal
in ImageMagick without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense,  and/or sell
copies of ImageMagick, and to permit persons to whom the ImageMagick is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of ImageMagick.

The software is provided "as is", without warranty of any kind, express or
implied, including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement.  In no event shall
ImageMagick Studio be liable for any claim, damages or other liability,
whether in an action of contract, tort or otherwise, arising from, out of or
in connection with ImageMagick or the use or other dealings in ImageMagick.

Except as contained in this notice, the name of the ImageMagick Studio shall
not be used in advertising or otherwise to promote the sale, use or other
dealings in ImageMagick without prior written authorization from the
ImageMagick Studio.
*/

#include "fmt_filters.h"
#include "Colors/helpers.h"

#include <cmath>
#include "cstring"

using namespace std;
#include <algorithm>

namespace fmt_filters
{

#define MaxRGB 255L
#define DegreesToRadians(x) ((x)*M_PI/180.0)
#define MagickSQ2PI 2.50662827463100024161235523934010416269302368164062
#define MagickEpsilon  1.0e-12
#define MagickPI  3.14159265358979323846264338327950288419716939937510

#define F_MAX(a, b)  ((b) < (a) ? (a) : (b))
#define F_MIN(a, b)  ((a) < (b) ? (a) : (b))

static void rgb2hsv(const fmt_filters::rgb &rgb, s32 *h, s32 *s, s32 *v);
static void hsv2rgb(s32 h, s32 s, s32 v, fmt_filters::rgb *rgb);
static fmt_filters::rgba interpolateColor(const fmt_filters::image &image, qreal x_offset, qreal y_offset, const fmt_filters::rgba &background);
static u32 generateNoise(u32 pixel, fmt_filters::NoiseType noise_type);
static u32 intensityValue(s32 r, s32 g, s32 b);
static u32 intensityValue(const fmt_filters::rgba &rr);
static s32 getBlurKernel(s32 width, qreal sigma, qreal **kernel);
static void blurScanLine(qreal *kernel, s32 width, fmt_filters::rgba *src, fmt_filters::rgba *dest, s32 columns);
static void hull(const s32 x_offset, const s32 y_offset, const s32 polarity, const s32 columns,
                        const s32 rows, u8 *f, u8 *g);
static bool convolveImage(fmt_filters::image *image, fmt_filters::rgba **dest, const unsigned int order, const qreal *kernel);
static int getOptimalKernelWidth(qreal radius, qreal sigma);

template<class T>
static void scaleDown(T &val, T min, T max);

struct double_packet
{
    qreal red;
    qreal green;
    qreal blue;
    qreal alpha;
};

struct short_packet
{
    unsigned short int red;
    unsigned short int green;
    unsigned short int blue;
    unsigned short int alpha;
};

bool checkImage(const image &im)
{
    return (im.rw && im.rh && im.w && im.h && im.data);
}

u8 qrealToU8(const qreal &val) {
    return (u8)qMin(255, qMax(0, qRound(val)));
}

#include "Colors/helpers.h"
// colorize tool
void colorizeHSV(const image &im,
              const qreal &hue,
              const qreal &saturation,
              const qreal &lightness,
              const qreal &alpha) {
    if(!checkImage(im)) return;

    u8 *bits;

    if(alpha > 0.001) {
        for(s32 y = 0; y < im.h; ++y) {
            bits = im.data + im.rw * y * sizeof(rgba);

            for(s32 x = 0; x < im.w; x++) {
                u8 u8aT = *(bits + 3);
                if(u8aT != 0) {
                    qreal aT = u8aT/255.;
                    u8 bT = *(bits);
                    u8 gT = *(bits + 1);
                    u8 rT = *(bits + 2);
                    u8 maxT = MAX3(rT, gT, bT);
                    u8 minT = MIN3(rT, gT, bT);

                    qreal hT = hue;
                    qreal sT = saturation;
                    qreal lT = (maxT + minT) / 510. + lightness;
                    qhsl_to_rgb(&hT, &sT, &lT);

                    *(bits) =
                        qrealToU8(
                            (alpha*lT*255. + (1. - alpha)*bT)*aT);
                    *(bits + 1) =
                        qrealToU8(
                            (alpha*sT*255. + (1. - alpha)*gT)*aT);
                    *(bits + 2) =
                        qrealToU8(
                            (alpha*hT*255. + (1. - alpha)*rT)*aT);
                }

                bits += 4;
            }
        }
    }
}

void replaceColor(const image &im,
              const int &redR,
              const int &greenR,
              const int &blueR,
              const int &alphaR,
              const int &redT,
              const int &greenT,
              const int &blueT,
              const int &alphaT,
              const int &tolerance,
              const qreal &smooth) {
    if(!checkImage(im)) return;

    u8 *bits;
    int dataW = im.w;
    int dataH = im.h;
    qreal *imMap = new qreal[dataW * dataH];

    for(s32 y = 0; y < im.h; ++y) {
        bits = im.data + im.rw * y * sizeof(rgba);

        for(s32 x = 0; x < im.w; x++) {
            int u8aT = (u8)*(bits + 3);
            int u8bT = (u8)*(bits);
            int u8gT = (u8)*(bits + 1);
            int u8rT = (u8)*(bits + 2);
            int avgDT = (qAbs(u8aT - alphaR) +
                         qAbs(u8bT - blueR) +
                         qAbs(u8gT - greenR) +
                         qAbs(u8rT - redR))/4;
            if(avgDT < tolerance) {
                imMap[y * dataW + x] = 1.;
            } else {
                imMap[y * dataW + x] = 0.;
            }

            bits += 4;
        }
    }

    qspredMono(imMap, dataW, dataH, smooth*10.);
    for(s32 y = 0; y < im.h; ++y) {
        bits = im.data + im.rw * y * sizeof(rgba);

        for(s32 x = 0; x < im.w; x++) {
            int u8aT = (u8)*(bits + 3);
            int u8bT = (u8)*(bits);
            int u8gT = (u8)*(bits + 1);
            int u8rT = (u8)*(bits + 2);
            qreal inf = imMap[y * dataW + x];
            if(inf > 0.001) {
                inf = qMin(1., 5*inf);
                *(bits) = qrealToU8(u8bT*(1. - inf) + blueT*inf);
                *(bits + 1) = qrealToU8(u8gT*(1. - inf) + greenT*inf);
                *(bits + 2) = qrealToU8(u8rT*(1. - inf) + redT*inf);
                *(bits + 3) = qrealToU8(u8aT*(1. - inf) + alphaT*inf);
            }

            bits += 4;
        }
    }

    delete[] imMap;
}

//void replaceColor(const image &im,
//              const int &redR,
//              const int &greenR,
//              const int &blueR,
//              const int &alphaR,
//              const int &redT,
//              const int &greenT,
//              const int &blueT,
//              const int &alphaT,
//              const int &tolerance,
//              const int &smooth) {
//    if(!checkImage(im)) return;

//    u8 *bits;

//    for(s32 y = 0; y < im.h; ++y) {
//        bits = im.data + im.rw * y * sizeof(rgba);

//        for(s32 x = 0; x < im.w; x++) {
//            int u8aT = *(bits + 3);
//            int u8bT = *(bits);
//            int u8gT = *(bits + 1);
//            int u8rT = *(bits + 2);
//            int avgDT = (qAbs(u8aT - alphaR) +
//                         qAbs(u8bT - blueR) +
//                         qAbs(u8gT - greenR) +
//                         qAbs(u8rT - redR))/4;
//            if(avgDT < tolerance) {
//                *(bits) = blueT;
//                *(bits + 1) = greenT;
//                *(bits + 2) = redT;
//                *(bits + 3) = alphaT;
//            } else if(avgDT < tolerance + smooth) {
//                qreal replaceAlpha = (tolerance + smooth - avgDT)/255.;
//                *(bits) = qrealToU8(
//                            (1. - replaceAlpha)*u8bT + replaceAlpha*blueT);
//                *(bits + 1) = qrealToU8(
//                            (1. - replaceAlpha)*u8gT + replaceAlpha*greenT);
//                *(bits + 2) = qrealToU8(
//                            (1. - replaceAlpha)*u8rT + replaceAlpha*redT);
//                *(bits + 3) = qrealToU8(
//                            (1. - replaceAlpha)*u8aT + replaceAlpha*alphaT);
//            }

//            bits += 4;
//        }
//    }
//}

// brightness tool
void anim_brightness(const image &im, qreal bn) {
    // check if all parameters are good
    if(!checkImage(im))
    return;

    u8 *bits;
    s32 val;

    // add to all color components 'bn' value, and check if the result is out of bounds.
    for(s32 y = 0; y < im.h; ++y) {
        bits = im.data + im.rw * y * sizeof(rgba);

        for(s32 x = 0; x < im.w; x++) {
            for(s32 v = 0; v < 3; v++) {
                val = bn + *bits;
                *bits = val < 0 ? 0 : (val > 255 ? 255 : (u8)val);

                bits++;
            }

            bits++;
        }
    }
}

// brightness tool
void brightness(const image &im, s32 bn) {
    // check if all parameters are good
    if(!checkImage(im))
    return;

    u8 *bits;
    s32 val;

    // add to all color components 'bn' value, and check if the result is out of bounds.
    for(s32 y = 0; y < im.h; ++y) {
        bits = im.data + im.rw * y * sizeof(rgba);

        for(s32 x = 0; x < im.w; x++) {
            for(s32 v = 0; v < 3; v++) {
                val = bn + *bits;
                *bits = val < 0 ? 0 : (val > 255 ? 255 : val);

                bits++;
            }

            bits++;
        }
    }
}

// gamma tool
void gamma(const image &im, qreal L) {
    // check if all parameters are good
    if(!checkImage(im))
    return;

    if(L == 0 || L < 0) L = 0.01;

    rgba *_rgba;
    u8 R, G, B;
    u8 GT[256];

    GT[0] = 0;

    // fill the array with gamma koefficients
    for (s32 x = 1; x < 256; ++x)
    GT[x] = (u8)round(255 * pow((double)x / 255.0, 1.0 / L));

    // now change gamma
    for(s32 y = 0;y < im.h;++y) {
        _rgba = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++) {
            R = _rgba[x].r;
            G = _rgba[x].g;
            B = _rgba[x].b;

            _rgba[x].r = GT[R];
            _rgba[x].g = GT[G];
            _rgba[x].b = GT[B];
        }
    }
}

void anim_contrast(const image &im, qreal contrast) {
    if(!checkImage(im) || !contrast) return;

    if(contrast < -255) contrast = -255;
    if(contrast >  255) contrast = 255;

    rgba *bits;
    u8 Ravg, Gavg, Bavg;
    s32 Ra = 0, Ga = 0, Ba = 0, Rn, Gn, Bn;

    // calculate the average values for RED, GREEN and BLUE
    // color components
    for(s32 y = 0; y < im.h; y++) {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++) {
            Ra += bits->r;
            Ga += bits->g;
            Ba += bits->b;

            bits++;
        }
    }

    s32 S = im.w * im.h;

    Ravg = Ra / S;
    Gavg = Ga / S;
    Bavg = Ba / S;

    for(s32 y = 0;y < im.h;y++) {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0; x < im.w; x++) {
            Rn = (contrast > 0) ? ((bits->r - Ravg) * 256 / (256 - contrast) + Ravg) : ((bits->r - Ravg) * (256 + contrast) / 256 + Ravg);
            Gn = (contrast > 0) ? ((bits->g - Gavg) * 256 / (256 - contrast) + Gavg) : ((bits->g - Gavg) * (256 + contrast) / 256 + Gavg);
            Bn = (contrast > 0) ? ((bits->b - Bavg) * 256 / (256 - contrast) + Bavg) : ((bits->b - Bavg) * (256 + contrast) / 256 + Bavg);

            bits->r = Rn < 0 ? 0 : (Rn > 255 ? 255 : Rn);
            bits->g = Gn < 0 ? 0 : (Gn > 255 ? 255 : Gn);
            bits->b = Bn < 0 ? 0 : (Bn > 255 ? 255 : Bn);

            bits++;
        }
    }
}

// contrast tool
void contrast(const image &im, s32 contrast) {
    if(!checkImage(im) || !contrast) return;

    if(contrast < -255) contrast = -255;
    if(contrast >  255) contrast = 255;

    rgba *bits;
    u8 Ravg, Gavg, Bavg;
    s32 Ra = 0, Ga = 0, Ba = 0, Rn, Gn, Bn;

    // calculate the average values for RED, GREEN and BLUE
    // color components
    for(s32 y = 0; y < im.h; y++) {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
        Ra += bits->r;
        Ga += bits->g;
        Ba += bits->b;

        bits++;
        }
    }

    s32 S = im.w * im.h;

    Ravg = Ra / S;
    Gavg = Ga / S;
    Bavg = Ba / S;

    // ok, now change contrast
    // with the terms of alghoritm:
    //
    // if contrast > 0: I = (I - Avg) * 256 / (256 - contrast) + Avg
    // if contrast < 0: I = (I - Avg) * (256 + contrast) / 256 + Avg
    //
    // where
    //   I - current color component value
    //   Avg - average value of this component (Ravg, Gavg or Bavg)
    //
    for(s32 y = 0;y < im.h;y++) {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0; x < im.w; x++) {
            Rn = (contrast > 0) ? ((bits->r - Ravg) * 256 / (256 - contrast) + Ravg) : ((bits->r - Ravg) * (256 + contrast) / 256 + Ravg);
            Gn = (contrast > 0) ? ((bits->g - Gavg) * 256 / (256 - contrast) + Gavg) : ((bits->g - Gavg) * (256 + contrast) / 256 + Gavg);
            Bn = (contrast > 0) ? ((bits->b - Bavg) * 256 / (256 - contrast) + Bavg) : ((bits->b - Bavg) * (256 + contrast) / 256 + Bavg);

            bits->r = Rn < 0 ? 0 : (Rn > 255 ? 255 : Rn);
            bits->g = Gn < 0 ? 0 : (Gn > 255 ? 255 : Gn);
            bits->b = Bn < 0 ? 0 : (Bn > 255 ? 255 : Bn);

            bits++;
        }
    }
}

// negative
void negative(const image &im)
{
    // check if all parameters are good
    if(!checkImage(im))
    return;

    rgba *bits;
    u8 R, G, B;

    for(s32 y = 0;y < im.h;y++)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
        R = bits->r;
        G = bits->g;
        B = bits->b;

        bits->r = 255 - R;
        bits->g = 255 - G;
        bits->b = 255 - B;

        bits++;
        }
    }
}

// swap RGB values
void swapRGB(const image &im, s32 type)
{
    // check if all parameters are good
    if(!checkImage(im) || (type != GBR && type != BRG))
    return;

    rgba *bits;
    u8 R, G, B;

    for(s32 y = 0;y < im.h;y++)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
        R = bits->r;
        G = bits->g;
        B = bits->b;

        bits->r = (type == GBR) ? G : B;
        bits->g = (type == GBR) ? B : R;
        bits->b = (type == GBR) ? R : G;

            bits++;
        }
    }
}

// blend
void blend(const image &im, const rgb &rgb, float opacity)
{
    // check parameters
    if(!checkImage(im))
    return;

    scaleDown(opacity, 0.0f, 1.0f);

    rgba *bits;
    s32 r = rgb.r, g = rgb.g, b = rgb.b;

    // blend!
    for(s32 y = 0;y < im.h;++y)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
            bits->r = bits->r + (u8)((b - bits->r) * opacity);
            bits->g = bits->g + (u8)((g - bits->g) * opacity);
            bits->b = bits->b + (u8)((r - bits->b) * opacity);

            bits++;
        }
    }
}

void replaceColor(const image &im,
                  const int &rInt,
                  const int &gInt,
                  const int &bInt) {
    // check parameters
    if(!checkImage(im))
    return;

    rgba *bits;
    unsigned char r = rInt;
    unsigned char g = gInt;
    unsigned char b = bInt;

    // blend!
    for(s32 y = 0; y < im.h; ++y) {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0; x < im.w; x++) {
            bits->r = b*bits->a/255;
            bits->g = g*bits->a/255;
            bits->b = r*bits->a/255;

            bits++;
        }
    }
}

void flatten(const image &im, const rgb &ca, const rgb &cb)
{
    if(!checkImage(im))
      return;

    s32 r1 = ca.r; s32 r2 = cb.r;
    s32 g1 = ca.g; s32 g2 = cb.g;
    s32 b1 = ca.b; s32 b2 = cb.b;
    s32 min = 0, max = 255;
    s32 mean;

    rgba *bits;
    rgb _rgb;

    for(s32 y = 0;y < im.h;++y)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;++x)
        {
            mean = (bits->r + bits->g + bits->b) / 3;
            min = F_MIN(min, mean);
            max = F_MAX(max, mean);
            bits++;
        }
    }

    // Conversion factors
    float sr = ((float) r2 - r1) / (max - min);
    float sg = ((float) g2 - g1) / (max - min);
    float sb = ((float) b2 - b1) / (max - min);

    // Repaint the image
    for(s32 y = 0;y < im.h;++y)
    {
        bits = (rgba *)im.data + im.w*y;

        for(s32 x = 0;x < im.w;++x)
        {
            mean = (bits->r + bits->g + bits->b) / 3;

            bits->r = (s32)(sr * (mean - min) + r1 + 0.5);
            bits->g = (s32)(sg * (mean - min) + g1 + 0.5);
            bits->b = (s32)(sb * (mean - min) + b1 + 0.5);

            bits++;
        }
    }
}

void fade(const image &im, const rgb &rgb, float val)
{
    if(!checkImage(im))
        return;

    u8 tbl[256];

    for (s32 i = 0;i < 256;i++)
        tbl[i] = (s32)(val * i + 0.5);

    s32 r, g, b, cr, cg, cb;

    rgba *bits;

    for(s32 y = 0;y < im.h;y++)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
            cr = bits->r;
            cg = bits->g;
            cb = bits->b;

            r = (cr > rgb.r) ? (cr - tbl[cr - rgb.r]) : (cr + tbl[rgb.r - cr]);
            g = (cg > rgb.g) ? (cg - tbl[cg - rgb.g]) : (cg + tbl[rgb.g - cg]);
            b = (cb > rgb.b) ? (cb - tbl[cb - rgb.b]) : (cb + tbl[rgb.b - cb]);

            bits->r = r;
            bits->g = g;
            bits->b = b;

            bits++;
        }
    }
}

void gray(const image &im)
{
    if(!checkImage(im))
        return;

    rgba *bits;
    s32 g;

    for(s32 y = 0;y < im.h;y++)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
            g = (bits->r * 11 + bits->g * 16 + bits->b * 5)/32;

            bits->r = g;
            bits->g = g;
            bits->b = g;

            bits++;
        }
    }
}

void desaturate(const image &im, float desat) {
    if(!checkImage(im))
      return;

    scaleDown(desat, 0.0f, 1.0f);

    rgba *bits;
    s32 h = 0, s = 0, v = 0;

    for(s32 y = 0;y < im.h;y++)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
            rgb _rgb(bits->r, bits->g, bits->b);
            rgb2hsv(_rgb, &h, &s, &v);
            hsv2rgb(h, (s32)(s * (1.0 - desat)), v, &_rgb);

            bits->r = _rgb.r;
            bits->g = _rgb.g;
            bits->b = _rgb.b;

            bits++;
        }
    }
}

void threshold(const image &im, u32 trh)
{
    if(!checkImage(im))
        return;

    scaleDown(trh, (u32)0, (u32)255);

    rgba *bits;

    for(s32 y = 0;y < im.h;y++)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
            if(intensityValue(bits->r, bits->g, bits->b) < trh)
                bits->r = bits->g = bits->b = 0;
            else
                bits->r = bits->g = bits->b = 255;

            bits++;
        }
    }
}

void solarize(const image &im, qreal factor) {
    if(!checkImage(im))
        return;

    s32 threshold;
    rgba *bits;

    threshold = (s32)(factor * (MaxRGB+1)/100.0);

    for(s32 y = 0;y < im.h;y++)
    {
        bits = (rgba *)im.data + im.rw * y;

        for(s32 x = 0;x < im.w;x++)
        {
            bits->r = bits->r > threshold ? MaxRGB-bits->r : bits->r;
            bits->g = bits->g > threshold ? MaxRGB-bits->g : bits->g;
            bits->b = bits->b > threshold ? MaxRGB-bits->b : bits->b;

            bits++;
        }
    }
}

void spread(const image &im, u32 amount) {
    if(!checkImage(im) || im.w < 3 || im.h < 3)
        return;

    rgba *n = new rgba [im.rw * im.rh];

    if(!n)
        return;

    s32 quantum;
    s32 x_distance, y_distance;
    rgba *bits = (rgba *)im.data, *q;

    memcpy(n, im.data, im.rw * im.rh * sizeof(rgba));

    quantum = (amount+1) >> 1;

    for(s32 y = 0;y < im.h;y++)
    {
        q = n + im.rw*y;

        for(s32 x = 0;x < im.w;x++)
        {
            x_distance = x + ((rand() & (amount+1))-quantum);
            y_distance = y + ((rand() & (amount+1))-quantum);
            x_distance = F_MIN(x_distance, im.w-1);
            y_distance = F_MIN(y_distance, im.h-1);

            if(x_distance < 0) x_distance = 0;
            if(y_distance < 0) y_distance = 0;

            *q++ = *(bits + y_distance*im.rw + x_distance);
        }
    }

    memcpy(im.data, n, im.rw * im.rh * sizeof(rgba));

    delete [] n;
}

void swirl(const image &im, qreal degrees, const rgba &background) {
    if(!checkImage(im))
        return;

    qreal cosine, distance, factor, radius, sine, x_center, x_distance,
            x_scale, y_center, y_distance, y_scale;
    s32 x, y;

    rgba *q, *p;
    rgba *bits = (rgba *)im.data;
    rgba *dest = new rgba [im.rw * im.rh];

    if(!dest)
        return;

    memcpy(dest, im.data, im.rw * im.rh * sizeof(rgba));

    // compute scaling factor
    x_center = im.w / 2.0;
    y_center = im.h / 2.0;

    radius = F_MAX(x_center, y_center);
    x_scale=1.0;
    y_scale=1.0;

    if(im.w > im.h)
        y_scale=(double)im.w / im.h;
    else if(im.w < im.h)
        x_scale=(double)im.h / im.w;

    degrees = DegreesToRadians(degrees);

    // swirl each row

    for(y = 0;y < im.h;y++)
    {
        p = bits + im.rw * y;
        q = dest + im.rw * y;
        y_distance = y_scale * (y-y_center);

        for(x = 0;x < im.w;x++)
        {
            // determine if the pixel is within an ellipse
            *q = *p;
            x_distance = x_scale*(x-x_center);
            distance = x_distance*x_distance+y_distance*y_distance;

            if(distance < (radius*radius))
            {
                // swirl
                factor = 1.0 - sqrt(distance)/radius;
                sine = sin(degrees*factor*factor);
                cosine = cos(degrees*factor*factor);

                *q = interpolateColor(im,
                                      (cosine*x_distance-sine*y_distance)/x_scale+x_center,
                                      (sine*x_distance+cosine*y_distance)/y_scale+y_center,
                                      background);
            }

            p++;
            q++;
        }
    }

    memcpy(im.data, dest, im.rw * im.rh * sizeof(rgba));

    delete [] dest;
}

void noise(const image &im, NoiseType noise_type)
{
    if(!checkImage(im))
        return;

    s32 x, y;
    rgba *dest = new rgba [im.rw * im.rh];

    if(!dest)
        return;

    rgba *bits;
    rgba *destData;

    for(y = 0;y < im.h;++y)
    {
        bits = (rgba *)im.data + im.rw * y;
        destData = dest + im.rw * y;

        for(x = 0;x < im.w;++x)
        {
            destData[x].r = generateNoise(bits->r, noise_type);
            destData[x].g = generateNoise(bits->g, noise_type);
            destData[x].b = generateNoise(bits->b, noise_type);
            destData[x].a = bits->a;

            bits++;
        }
    }

    memcpy(im.data, dest, im.rw * im.rh * sizeof(rgba));

    delete [] dest;
}

void implode(const image &im, qreal _factor, const rgba &background)
{
    if(!checkImage(im))
        return;

    qreal amount, distance, radius;
    qreal x_center, x_distance, x_scale;
    qreal y_center, y_distance, y_scale;
    rgba *dest;
    s32 x, y;

    rgba *n = new rgba [im.rw * im.rh];

    if(!n)
        return;

    rgba *bits;

    // compute scaling factor
    x_scale = 1.0;
    y_scale = 1.0;
    x_center = (double)0.5*im.w;
    y_center = (double)0.5*im.h;
    radius=x_center;

    if(im.w > im.h)
        y_scale = (double)im.w/im.h;
    else if(im.w < im.h)
    {
        x_scale = (double)im.h/im.w;
        radius = y_center;
    }

    amount=_factor/10.0;

    if(amount >= 0)
        amount/=10.0;

    qreal factor;

    for(y = 0;y < im.h;++y)
    {
        bits = (rgba *)im.data + im.rw * y;
        dest =  n + im.rw * y;

        y_distance = y_scale * (y-y_center);

        for(x = 0;x < im.w;++x)
        {
            x_distance = x_scale*(x-x_center);
            distance= x_distance*x_distance+y_distance*y_distance;

            if(distance < (radius*radius))
            {
                // Implode the pixel.
                factor = 1.0;

                if(distance > 0.0)
                    factor = pow(sin(0.5000000000000001*M_PI*sqrt(distance)/radius),-amount);

                *dest = interpolateColor(im, factor*x_distance/x_scale+x_center,
                                           factor*y_distance/y_scale+y_center,
                                           background);
            }
            else
                *dest = *bits;

            bits++;
            dest++;
        }
    }

    memcpy(im.data, n, im.rw * im.rh * sizeof(rgba));

    delete [] n;
}

void despeckle(const image &im)
{
    if(!checkImage(im))
        return;

    s32 i, j, x, y;
    u8 *blue_channel, *red_channel, *green_channel, *buffer, *alpha_channel;
    s32 packets;

    static const s32
                    X[4] = {0, 1, 1,-1},
                    Y[4] = {1, 0, 1, 1};

    rgba *n = new rgba [im.rw * im.rh];

    if(!n)
        return;

    packets = (im.w+2) * (im.h+2);

    red_channel = new u8 [packets];
    green_channel = new u8 [packets];
    blue_channel = new u8 [packets];
    alpha_channel = new u8 [packets];
    buffer = new u8 [packets];

    if(!red_channel || ! green_channel || ! blue_channel || ! alpha_channel || !buffer)
    {
        if(red_channel)   delete [] red_channel;
        if(green_channel) delete [] green_channel;
        if(blue_channel)  delete [] blue_channel;
        if(alpha_channel) delete [] alpha_channel;
        if(buffer)        delete [] buffer;

        delete [] n;

        return;
    }

    // copy image pixels to color component buffers
    j = im.w+2;

    rgba *bits;

    for(y = 0;y < im.h;++y)
    {
        bits = (rgba *)im.data + im.rw*y;
        ++j;

        for(x = 0;x < im.w;++x)
        {
            red_channel[j] = bits->r;
            green_channel[j] = bits->g;
            blue_channel[j] = bits->b;
            alpha_channel[j] = bits->a;

            bits++;
            ++j;
        }

        ++j;
    }

    // reduce speckle in red channel
    for(i = 0;i < 4;i++)
    {
        hull(X[i],Y[i],1,im.w,im.h,red_channel,buffer);
        hull(-X[i],-Y[i],1,im.w,im.h,red_channel,buffer);
        hull(-X[i],-Y[i],-1,im.w,im.h,red_channel,buffer);
        hull(X[i],Y[i],-1,im.w,im.h,red_channel,buffer);
    }

    // reduce speckle in green channel
    for(i = 0;i < packets;i++)
        buffer[i] = 0;

    for(i = 0;i < 4;i++)
    {
        hull(X[i],Y[i],1,im.w,im.h,green_channel,buffer);
        hull(-X[i],-Y[i],1,im.w,im.h,green_channel,buffer);
        hull(-X[i],-Y[i],-1,im.w,im.h,green_channel,buffer);
        hull(X[i],Y[i],-1,im.w,im.h,green_channel,buffer);
    }

    // reduce speckle in blue channel
    for(i = 0;i < packets;i++)
        buffer[i] = 0;

    for(i = 0;i < 4;i++)
    {
        hull(X[i],Y[i],1,im.w,im.h,blue_channel,buffer);
        hull(-X[i],-Y[i],1,im.w,im.h,blue_channel,buffer);
        hull(-X[i],-Y[i],-1,im.w,im.h,blue_channel,buffer);
        hull(X[i],Y[i],-1,im.w,im.h,blue_channel,buffer);
    }

    // copy color component buffers to despeckled image
    j = im.w+2;

    for(y = 0;y < im.h;++y)
    {
        bits = n + im.rw*y;
        ++j;

        for(x = 0;x < im.w;++x)
        {
            *bits = rgba(red_channel[j], green_channel[j], blue_channel[j], alpha_channel[j]);

            bits++;
            ++j;
        }

        ++j;
    }

    delete [] buffer;
    delete [] red_channel;
    delete [] green_channel;
    delete [] blue_channel;
    delete [] alpha_channel;

    memcpy(im.data, n, im.rw * im.rh * sizeof(rgba));

    delete [] n;
}

void anim_fast_blur(const image &im,
                    const qreal &fRadius,
                    const qreal &opacityT) {
    if(fRadius < 0.01) return;
    unsigned char *pix = im.data;
    int w = im.w;
    int h = im.h;

    qreal divF = fRadius + fRadius + 1.;
    qreal divFInv = 1./divF;
    int iRadius = ceil(fRadius);
    int nPoints = iRadius + iRadius + 1;
    qreal fracInf = 1. - iRadius + fRadius;
    qreal fracInfInv = 1. - fracInf;

    qreal *rLine = new double[nPoints];
    qreal *gLine = new double[nPoints];
    qreal *bLine = new double[nPoints];
    qreal *aLine = new double[nPoints];

    int wm=w-1;
    int hm=h-1;
    int wh=w*h;
    qreal *r=new double[wh];
    qreal *g=new double[wh];
    qreal *b=new double[wh];
    qreal *a=new double[wh];
    qreal rsum,gsum,bsum,asum;
    int x,y,i,p,p1,yp,yi,yw;
    int *vMIN = new int[qMax(w,h)];
    int *vMAX = new int[qMax(w,h)];


    yw=yi=0;

    for (y=0;y<h;y++){
        p = yi * 4;
        rLine[0] = pix[p];
        rsum = pix[p]*fracInf;
        gLine[0] = pix[p + 1];
        gsum = pix[p + 1]*fracInf;
        bLine[0] = pix[p + 2];
        bsum = pix[p + 2]*fracInf;
        aLine[0] = pix[p + 3]*opacityT;
        asum = pix[p + 3]*fracInf*opacityT;

        for(i = 1 - iRadius; i < iRadius ; i++){
            p = (yi + qMin(wm, qMax(i,0))) * 4;
            rLine[i + iRadius] = pix[p];
            rsum += pix[p];
            gLine[i + iRadius] = pix[p + 1];
            gsum += pix[p + 1];
            bLine[i + iRadius] = pix[p + 2];
            bsum += pix[p + 2];
            aLine[i + iRadius] = pix[p + 3]*opacityT;
            asum += pix[p + 3]*opacityT;
        }

        p = (yi + qMin(wm, iRadius)) * 4;
        rLine[iRadius + iRadius] = pix[p];
        rsum += pix[p]*fracInf;
        gLine[iRadius + iRadius] = pix[p + 1];
        gsum += pix[p + 1]*fracInf;
        bLine[iRadius + iRadius] = pix[p + 2];
        bsum += pix[p + 2]*fracInf;
        aLine[iRadius + iRadius] = pix[p + 3]*opacityT;
        asum += pix[p + 3]*fracInf*opacityT;

        for (x = 0; x < w; x++){

            r[yi] = rsum*divFInv;
            g[yi] = gsum*divFInv;
            b[yi] = bsum*divFInv;
            a[yi] = asum*divFInv;

            if(y == 0) {
                vMIN[x]=qMin(x+iRadius+1,wm);
                vMAX[x]=qMax(x-iRadius,0);
            }
            p1 = (yw + vMIN[x])*4;

            rsum -= rLine[0]*fracInf;
            gsum -= gLine[0]*fracInf;
            bsum -= bLine[0]*fracInf;
            asum -= aLine[0]*fracInf;

            rsum -= rLine[1]*fracInfInv;
            gsum -= gLine[1]*fracInfInv;
            bsum -= bLine[1]*fracInfInv;
            asum -= aLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                rLine[i] = rLine[i + 1];
                gLine[i] = gLine[i + 1];
                bLine[i] = bLine[i + 1];
                aLine[i] = aLine[i + 1];
            }

            rsum += rLine[nPoints - 1]*fracInfInv;
            gsum += gLine[nPoints - 1]*fracInfInv;
            bsum += bLine[nPoints - 1]*fracInfInv;
            asum += aLine[nPoints - 1]*fracInfInv;

            rLine[nPoints - 1] = pix[p1];
            gLine[nPoints - 1] = pix[p1 + 1];
            bLine[nPoints - 1] = pix[p1 + 2];
            aLine[nPoints - 1] = pix[p1 + 3]*opacityT;

            rsum += pix[p1]*fracInf;
            gsum += pix[p1 + 1]*fracInf;
            bsum += pix[p1 + 2]*fracInf;
            asum += pix[p1 + 3]*fracInf*opacityT;

            yi++;
        }
        yw+=w;
    }

    for (x=0;x<w;x++){
        yp=-iRadius*w;

        yi=qMax(0,yp)+x;
        rLine[0] = r[yi];
        rsum = r[yi]*fracInf;
        gLine[0] = g[yi];
        gsum = g[yi]*fracInf;
        bLine[0] = b[yi];
        bsum = b[yi]*fracInf;
        aLine[0] = a[yi];
        asum = a[yi]*fracInf;
        yp+=w;

        for(i = 1 - iRadius; i < iRadius ; i++){
            yi=qMax(0,yp)+x;
            rLine[i + iRadius] = r[yi];
            rsum += r[yi];
            gLine[i + iRadius] = g[yi];
            gsum += g[yi];
            bLine[i + iRadius] = b[yi];
            bsum += b[yi];
            aLine[i + iRadius] = a[yi];
            asum += a[yi];
            yp+=w;
        }

        yi=qMax(0,yp)+x;
        rLine[iRadius + iRadius] = r[yi];
        rsum += r[yi]*fracInf;
        gLine[iRadius + iRadius] = g[yi];
        gsum += g[yi]*fracInf;
        bLine[iRadius + iRadius] = b[yi];
        bsum += b[yi]*fracInf;
        aLine[iRadius + iRadius] = a[yi];
        asum += a[yi]*fracInf;
        yp+=w;


        yi=x;
        for (y=0;y<h;y++){
            unsigned char aVal = qMin(255, qMax(0, (int)(asum*divFInv)));
            pix[yi*4]		= qMin(aVal,
                                  (unsigned char)qMin(255,
                                      qMax(0, (int)(rsum*divFInv))));
            pix[yi*4 + 1]	= qMin(aVal,
                                  (unsigned char)qMin(255,
                                      qMax(0, (int)(gsum*divFInv))));
            pix[yi*4 + 2]	= qMin(aVal,
                                  (unsigned char)qMin(255,
                                      qMax(0, (int)(bsum*divFInv))));
            pix[yi*4 + 3]	= aVal;
            if(x==0) {
                vMIN[y]=qMin(y+iRadius+1,hm)*w;
                vMAX[y]=qMax(y-iRadius,0)*w;
            }
            p1=x+vMIN[y];

            rsum -= rLine[0]*fracInf;
            gsum -= gLine[0]*fracInf;
            bsum -= bLine[0]*fracInf;
            asum -= aLine[0]*fracInf;

            rsum -= rLine[1]*fracInfInv;
            gsum -= gLine[1]*fracInfInv;
            bsum -= bLine[1]*fracInfInv;
            asum -= aLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                rLine[i] = rLine[i + 1];
                gLine[i] = gLine[i + 1];
                bLine[i] = bLine[i + 1];
                aLine[i] = aLine[i + 1];
            }

            rsum += rLine[nPoints - 1]*fracInfInv;
            gsum += gLine[nPoints - 1]*fracInfInv;
            bsum += bLine[nPoints - 1]*fracInfInv;
            asum += aLine[nPoints - 1]*fracInfInv;

            rLine[nPoints - 1] = r[p1];
            gLine[nPoints - 1] = g[p1];
            bLine[nPoints - 1] = b[p1];
            aLine[nPoints - 1] = a[p1];

            rsum += r[p1]*fracInf;
            gsum += g[p1]*fracInf;
            bsum += b[p1]*fracInf;
            asum += a[p1]*fracInf;

            yi+=w;
        }
    }

    delete[] rLine;
    delete[] gLine;
    delete[] bLine;
    delete[] aLine;

    delete[] r;
    delete[] g;
    delete[] b;
    delete[] a;

    delete[] vMIN;
    delete[] vMAX;
}

void anim_fast_blur(const image &im,
                    const qreal &fRadius) {
    if(fRadius < 0.01) return;
    unsigned char *pix = im.data;
    int w = im.w;
    int h = im.h;

    qreal divF = fRadius + fRadius + 1.;
    qreal divFInv = 1./divF;
    int iRadius = ceil(fRadius);
    int nPoints = iRadius + iRadius + 1;
    qreal fracInf = 1. - iRadius + fRadius;
    qreal fracInfInv = 1. - fracInf;

    qreal *rLine = new double[nPoints];
    qreal *gLine = new double[nPoints];
    qreal *bLine = new double[nPoints];
    qreal *aLine = new double[nPoints];

    int wm=w-1;
    int hm=h-1;
    int wh=w*h;
    qreal *r=new double[wh];
    qreal *g=new double[wh];
    qreal *b=new double[wh];
    qreal *a=new double[wh];
    qreal rsum,gsum,bsum,asum;
    int x,y,i,p,p1,yp,yi,yw;
    int *vMIN = new int[qMax(w,h)];
    int *vMAX = new int[qMax(w,h)];


    yw=yi=0;

    for (y=0;y<h;y++){
        p = yi * 4;
        rLine[0] = pix[p];
        rsum = pix[p]*fracInf;
        gLine[0] = pix[p + 1];
        gsum = pix[p + 1]*fracInf;
        bLine[0] = pix[p + 2];
        bsum = pix[p + 2]*fracInf;
        aLine[0] = pix[p + 3];
        asum = pix[p + 3]*fracInf;

        for(i = 1 - iRadius; i < iRadius ; i++){
            p = (yi + qMin(wm, qMax(i,0))) * 4;
            rLine[i + iRadius] = pix[p];
            rsum += pix[p];
            gLine[i + iRadius] = pix[p + 1];
            gsum += pix[p + 1];
            bLine[i + iRadius] = pix[p + 2];
            bsum += pix[p + 2];
            aLine[i + iRadius] = pix[p + 3];
            asum += pix[p + 3];
        }

        p = (yi + qMin(wm, iRadius)) * 4;
        rLine[iRadius + iRadius] = pix[p];
        rsum += pix[p]*fracInf;
        gLine[iRadius + iRadius] = pix[p + 1];
        gsum += pix[p + 1]*fracInf;
        bLine[iRadius + iRadius] = pix[p + 2];
        bsum += pix[p + 2]*fracInf;
        aLine[iRadius + iRadius] = pix[p + 3];
        asum += pix[p + 3]*fracInf;

        for (x = 0; x < w; x++){

            r[yi] = rsum*divFInv;
            g[yi] = gsum*divFInv;
            b[yi] = bsum*divFInv;
            a[yi] = asum*divFInv;

            if(y == 0) {
                vMIN[x]=qMin(x+iRadius+1,wm);
                vMAX[x]=qMax(x-iRadius,0);
            }
            p1 = (yw + vMIN[x])*4;

            rsum -= rLine[0]*fracInf;
            gsum -= gLine[0]*fracInf;
            bsum -= bLine[0]*fracInf;
            asum -= aLine[0]*fracInf;

            rsum -= rLine[1]*fracInfInv;
            gsum -= gLine[1]*fracInfInv;
            bsum -= bLine[1]*fracInfInv;
            asum -= aLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                rLine[i] = rLine[i + 1];
                gLine[i] = gLine[i + 1];
                bLine[i] = bLine[i + 1];
                aLine[i] = aLine[i + 1];
            }

            rsum += rLine[nPoints - 1]*fracInfInv;
            gsum += gLine[nPoints - 1]*fracInfInv;
            bsum += bLine[nPoints - 1]*fracInfInv;
            asum += aLine[nPoints - 1]*fracInfInv;

            rLine[nPoints - 1] = pix[p1];
            gLine[nPoints - 1] = pix[p1 + 1];
            bLine[nPoints - 1] = pix[p1 + 2];
            aLine[nPoints - 1] = pix[p1 + 3];

            rsum += pix[p1]*fracInf;
            gsum += pix[p1 + 1]*fracInf;
            bsum += pix[p1 + 2]*fracInf;
            asum += pix[p1 + 3]*fracInf;

            yi++;
        }
        yw+=w;
    }

    for (x=0;x<w;x++){
        yp=-iRadius*w;

        yi=qMax(0,yp)+x;
        rLine[0] = r[yi];
        rsum = r[yi]*fracInf;
        gLine[0] = g[yi];
        gsum = g[yi]*fracInf;
        bLine[0] = b[yi];
        bsum = b[yi]*fracInf;
        aLine[0] = a[yi];
        asum = a[yi]*fracInf;
        yp+=w;

        for(i = 1 - iRadius; i < iRadius ; i++){
            yi=qMax(0,yp)+x;
            rLine[i + iRadius] = r[yi];
            rsum += r[yi];
            gLine[i + iRadius] = g[yi];
            gsum += g[yi];
            bLine[i + iRadius] = b[yi];
            bsum += b[yi];
            aLine[i + iRadius] = a[yi];
            asum += a[yi];
            yp+=w;
        }

        yi=qMax(0,yp)+x;
        rLine[iRadius + iRadius] = r[yi];
        rsum += r[yi]*fracInf;
        gLine[iRadius + iRadius] = g[yi];
        gsum += g[yi]*fracInf;
        bLine[iRadius + iRadius] = b[yi];
        bsum += b[yi]*fracInf;
        aLine[iRadius + iRadius] = a[yi];
        asum += a[yi]*fracInf;
        yp+=w;


        yi=x;
        for (y=0;y<h;y++){
            unsigned char aVal = qMin(255, qMax(0, (int)(asum*divFInv)));
            pix[yi*4]		= qMin(aVal,
                                  (unsigned char)qMin(255,
                                      qMax(0, (int)(rsum*divFInv))));
            pix[yi*4 + 1]	= qMin(aVal,
                                  (unsigned char)qMin(255,
                                      qMax(0, (int)(gsum*divFInv))));
            pix[yi*4 + 2]	= qMin(aVal,
                                  (unsigned char)qMin(255,
                                      qMax(0, (int)(bsum*divFInv))));
            pix[yi*4 + 3]	= aVal;
            if(x==0) {
                vMIN[y]=qMin(y+iRadius+1,hm)*w;
                vMAX[y]=qMax(y-iRadius,0)*w;
            }
            p1=x+vMIN[y];

            rsum -= rLine[0]*fracInf;
            gsum -= gLine[0]*fracInf;
            bsum -= bLine[0]*fracInf;
            asum -= aLine[0]*fracInf;

            rsum -= rLine[1]*fracInfInv;
            gsum -= gLine[1]*fracInfInv;
            bsum -= bLine[1]*fracInfInv;
            asum -= aLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                rLine[i] = rLine[i + 1];
                gLine[i] = gLine[i + 1];
                bLine[i] = bLine[i + 1];
                aLine[i] = aLine[i + 1];
            }

            rsum += rLine[nPoints - 1]*fracInfInv;
            gsum += gLine[nPoints - 1]*fracInfInv;
            bsum += bLine[nPoints - 1]*fracInfInv;
            asum += aLine[nPoints - 1]*fracInfInv;

            rLine[nPoints - 1] = r[p1];
            gLine[nPoints - 1] = g[p1];
            bLine[nPoints - 1] = b[p1];
            aLine[nPoints - 1] = a[p1];

            rsum += r[p1]*fracInf;
            gsum += g[p1]*fracInf;
            bsum += b[p1]*fracInf;
            asum += a[p1]*fracInf;

            yi+=w;
        }
    }

    delete[] rLine;
    delete[] gLine;
    delete[] bLine;
    delete[] aLine;

    delete[] r;
    delete[] g;
    delete[] b;
    delete[] a;

    delete[] vMIN;
    delete[] vMAX;
}

void anim_fast_shadow(const image &im,
                      const qreal &fRed,
                      const qreal &fGreen,
                      const qreal &fBlue,
                      const qreal &fDx,
                      const qreal &fDy,
                      const qreal &fRadius) {
    unsigned char *pix = im.data;
    int w = im.w;
    int h = im.h;

    int iDx = floor(fDx);
    int iDy = floor(fDy);

    qreal divF = fRadius + fRadius + 1.;
    qreal divFInv = 1./divF;
    int iRadius = ceil(fRadius);
    int nPoints = iRadius + iRadius + 1;
    qreal fracInf = 1. - iRadius + fRadius;
    qreal fracInfInv = 1. - fracInf;

    qreal *aLine = new double[nPoints];

    int wh=w*h;
    qreal *a = new double[wh];
    qreal asum;
    int x,y,i,p,p1,yp,yi,yw,dp;
    int *vMIN = new int[qMax(w,h)];
    int *vMAX = new int[qMax(w,h)];
    int wm=w-1;
    int hm=h-1;

    yw = yi = 0;

    int yMin = qMax(iDy, 0);
    int yMax = qMin(h + iDy, h);
    int xMin = qMax(iDx, 0);
    int xMax = qMin(w + iDx, w);
    dp = -iDx - iDy*w;

    yi += yMin*w;
    yw += yMin*w;

    for(y = yMin; y < yMax; y++) {
        yi += xMin;
        p = (yi + dp) * 4;
        aLine[0] = pix[p + 3];
        asum = pix[p + 3]*fracInf;

        for(i = 1 - iRadius; i < iRadius ; i++){
            p = (yi + qMin(wm, qMax(i,0)) + dp) * 4;
            aLine[i + iRadius] = pix[p + 3];
            asum += pix[p + 3];
        }

        p = (yi + qMin(wm, iRadius) + dp) * 4;
        aLine[iRadius + iRadius] = pix[p + 3];
        asum += pix[p + 3]*fracInf;

        for(x = xMin; x < xMax; x++) {
            a[yi] = asum*divFInv;

            if(y == yMin) {
                vMIN[x] = qMin(x + iRadius + 1, wm);
                vMAX[x] = qMax(x - iRadius, 0);
            }
            p1 = (yw + vMIN[x] + dp)*4;

            asum -= aLine[0]*fracInf;

            asum -= aLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                aLine[i] = aLine[i + 1];
            }

            asum += aLine[nPoints - 1]*fracInfInv;

            aLine[nPoints - 1] = pix[p1 + 3];

            asum += pix[p1 + 3]*fracInf;

            yi++;
        }
        yi += w - xMax;
        yw += w;
    }

    for(x = xMin; x < xMax; x++) {
        yp=-iRadius*w;

        yi=qMax(0,yp)+x;
        aLine[0] = a[yi];
        asum = a[yi]*fracInf;
        yp += w;

        for(i = 1 - iRadius; i < iRadius; i++){
            yi = qMax(0, yp)+x;
            aLine[i + iRadius] = a[yi];
            asum += a[yi];
            yp+=w;
        }

        yi = qMax(0, yp)+x;
        aLine[iRadius + iRadius] = a[yi];
        asum += a[yi] * fracInf;
        yp += w;


        yi = x;
        yi += yMin*w;
        for(y = yMin; y < yMax; y++) {
            int pixA = pix[yi*4 + 3];
            if(pixA != 255) {
                qreal pixAFrac = pixA/255.;
                qreal shadowAFrac = asum*divFInv/255.;
                qreal aMult = shadowAFrac*pixAFrac;
                qreal fAVal = (shadowAFrac + pixAFrac - aMult)*255.;
                unsigned char aVal = qMin(255,
                                         qMax(0,
                                             qRound(fAVal)) );
                unsigned char pixR = pix[yi*4];
                int iRVal = round((pixR*aMult +
                            pixR*(1. - shadowAFrac) +
                            fRed*255*(1. - pixAFrac))*fAVal/255.);
                pix[yi*4] = qMin(aVal,
                                      (unsigned char)qMin(255,
                                          qMax(0, iRVal)));
                unsigned char pixG = pix[yi*4 + 1];
                int iGVal = round((pixG*aMult +
                            pixG*(1. - shadowAFrac) +
                            fGreen*255*(1. - pixAFrac))*fAVal/255.);
                pix[yi*4 + 1] = qMin(aVal,
                                      (unsigned char)qMin(255,
                                          qMax(0, iGVal)));
                unsigned char pixB = pix[yi*4 + 2];
                int iBVal = round((pixB*aMult +
                            pixB*(1. - shadowAFrac) +
                            fBlue*255*(1. - pixAFrac))*fAVal/255.);
                pix[yi*4 + 2] = qMin(aVal,
                                      (unsigned char)qMin(255,
                                          qMax(0, iBVal)));
                pix[yi*4 + 3] = aVal;
            }

            if(x == xMin) {
                vMIN[y] = qMin(y + iRadius + 1, hm) * w;
                vMAX[y] = qMax(y - iRadius, 0) * w;
            }
            p1 = x + vMIN[y];

            asum -= aLine[0]*fracInf;

            asum -= aLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                aLine[i] = aLine[i + 1];
            }

            asum += aLine[nPoints - 1]*fracInfInv;

            aLine[nPoints - 1] = a[p1];

            asum += a[p1]*fracInf;

            yi += w;
        }
    }

    delete[] aLine;

    delete[] a;

    delete[] vMIN;
    delete[] vMAX;
}

//void fast_blur(const image &im, qreal radiusF)
//{
//    unsigned char *pix = im.data;
//    int w = im.w;
//    int h = im.h;

//    int maxRadius = ceil(radiusF);
//    qreal fraqInf = maxRadius - radiusF;

//    if (maxRadius<1) return;
//    int wm=w-1;
//    int hm=h-1;
//    int wh=w*h;
//    int div=maxRadius+maxRadius+1;
//    unsigned char *r=new unsigned char[wh];
//    unsigned char *g=new unsigned char[wh];
//    unsigned char *b=new unsigned char[wh];
//    unsigned char *a=new unsigned char[wh];
//    int rsum,gsum,bsum,asum,x,y,i,p,p1,p2,yp,yi,yw;
//    int *vMIN = new int[qMax(w,h)];
//    int *vMAX = new int[qMax(w,h)];

//    qreal divF = radiusF + radiusF + 1.;
//    int minFi = 256*div - ceil(256*divF);
//    qreal *dv=new double[256*div];
//    for(i=0; i< minFi; i++) {
//        dv[i] = 0.;
//    }
//    for (i=minFi;i<256*div;i++) {
//        dv[i]= ((i - minFi)/divF);
//        printf("%4.2f ", dv[i]);
//    }

//    yw=yi=0;

//    for (y=0;y<h;y++){
//        rsum=gsum=bsum=asum=0;

//        for(i=-maxRadius;i<=maxRadius;i++){
//            p = (yi + qMin(wm, qMax(i,0))) * 4;
//            rsum += pix[p];
//            gsum += pix[p+1];
//            bsum += pix[p+2];
//            asum += pix[p+3];
//        }
//        for (x=0;x<w;x++){

//            r[yi]=dv[rsum];
//            g[yi]=dv[gsum];
//            b[yi]=dv[bsum];
//            a[yi]=dv[asum];

//            if(y==0){
//                vMIN[x]=qMin(x+maxRadius+1,wm);
//                vMAX[x]=qMax(x-maxRadius,0);
//            }
//            p1 = (yw+vMIN[x])*4;
//            p2 = (yw+vMAX[x])*4;

//            rsum += pix[p1]		- pix[p2];
//            gsum += pix[p1+1]	- pix[p2+1];
//            bsum += pix[p1+2]	- pix[p2+2];
//            asum += pix[p1+3]	- pix[p2+3];

//            yi++;
//        }
//        yw+=w;
//    }

//    for (x=0;x<w;x++){
//        rsum=gsum=bsum=asum=0;
//        yp=-maxRadius*w;
//        for(i=-maxRadius;i<=maxRadius;i++){
//            yi=qMax(0,yp)+x;
//            rsum+=r[yi];
//            gsum+=g[yi];
//            bsum+=b[yi];
//            asum+=a[yi];
//            yp+=w;
//        }
//        yi=x;
//        for (y=0;y<h;y++){
//            pix[yi*4]		= dv[rsum];
//            pix[yi*4 + 1]	= dv[gsum];
//            pix[yi*4 + 2]	= dv[bsum];
//            pix[yi*4 + 3]	= dv[asum];
//            if(x==0){
//                vMIN[y]=qMin(y+maxRadius+1,hm)*w;
//                vMAX[y]=qMax(y-maxRadius,0)*w;
//            }
//            p1=x+vMIN[y];
//            p2=x+vMAX[y];

//            rsum+=r[p1]-r[p2];
//            gsum+=g[p1]-g[p2];
//            bsum+=b[p1]-b[p2];
//            asum+=a[p1]-a[p2];

//            yi+=w;
//        }
//    }

//    delete r;
//    delete g;
//    delete b;
//    delete a;

//    delete vMIN;
//    delete vMAX;
//    delete dv;
//}



void fast_blur(const image &im, int radius,
               const qreal &opacityT) {
    unsigned char *pix = im.data;
    int w = im.w;
    int h = im.h;

    if (radius<1) return;
    int wm=w-1;
    int hm=h-1;
    int wh=w*h;
    int div=radius+radius+1;
    unsigned char *r=new unsigned char[wh];
    unsigned char *g=new unsigned char[wh];
    unsigned char *b=new unsigned char[wh];
    unsigned char *a=new unsigned char[wh];
    int rsum,gsum,bsum,asum,x,y,i,p,p1,p2,yp,yi,yw;
    int *vMIN = new int[qMax(w,h)];
    int *vMAX = new int[qMax(w,h)];

    int nDiv = 256*div;
    unsigned char *dv=new unsigned char[nDiv];
    int nDivMinus1 = nDiv - 1;
    for (i=0;i<256*div;i++) dv[i]=(i/div);

    yw=yi=0;

    for (y=0;y<h;y++){
        rsum=gsum=bsum=asum=0;
        for(i=-radius;i<=radius;i++){
            p = (yi + qMin(wm, qMax(i,0))) * 4;
            rsum += pix[p];
            gsum += pix[p+1];
            bsum += pix[p+2];
            asum += pix[p+3];
        }
        for (x=0;x<w;x++){

            r[yi]=dv[rsum];
            g[yi]=dv[gsum];
            b[yi]=dv[bsum];
            a[yi]=dv[qMin(nDivMinus1, (int)(asum*opacityT))];

            if(y==0){
                vMIN[x]=qMin(x+radius+1,wm);
                vMAX[x]=qMax(x-radius,0);
            }
            p1 = (yw+vMIN[x])*4;
            p2 = (yw+vMAX[x])*4;

            rsum += pix[p1]		- pix[p2];
            gsum += pix[p1+1]	- pix[p2+1];
            bsum += pix[p1+2]	- pix[p2+2];
            asum += pix[p1+3]	- pix[p2+3];

            yi++;
        }
        yw+=w;
    }

    for (x=0;x<w;x++){
        rsum=gsum=bsum=asum=0;
        yp=-radius*w;
        for(i=-radius;i<=radius;i++){
            yi=qMax(0,yp)+x;
            rsum+=r[yi];
            gsum+=g[yi];
            bsum+=b[yi];
            asum+=a[yi];
            yp+=w;
        }
        yi=x;
        for (y=0;y<h;y++){
            pix[yi*4]		= dv[rsum];
            pix[yi*4 + 1]	= dv[gsum];
            pix[yi*4 + 2]	= dv[bsum];
            pix[yi*4 + 3]	= dv[asum];
            if(x==0){
                vMIN[y]=qMin(y+radius+1,hm)*w;
                vMAX[y]=qMax(y-radius,0)*w;
            }
            p1=x+vMIN[y];
            p2=x+vMAX[y];

            rsum+=r[p1]-r[p2];
            gsum+=g[p1]-g[p2];
            bsum+=b[p1]-b[p2];
            asum+=a[p1]-a[p2];

            yi+=w;
        }
    }

    delete[] r;
    delete[] g;
    delete[] b;
    delete[] a;

    delete[] vMIN;
    delete[] vMAX;
    delete[] dv;
}

void fast_blur(const image &im, int radius)
{
    unsigned char *pix = im.data;
    int w = im.w;
    int h = im.h;

    if (radius<1) return;
    int wm=w-1;
    int hm=h-1;
    int wh=w*h;
    int div=radius+radius+1;
    unsigned char *r=new unsigned char[wh];
    unsigned char *g=new unsigned char[wh];
    unsigned char *b=new unsigned char[wh];
    unsigned char *a=new unsigned char[wh];
    int rsum,gsum,bsum,asum,x,y,i,p,p1,p2,yp,yi,yw;
    int *vMIN = new int[qMax(w,h)];
    int *vMAX = new int[qMax(w,h)];

    unsigned char *dv=new unsigned char[256*div];
    for (i=0;i<256*div;i++) dv[i]=(i/div);

    yw=yi=0;

    for (y=0;y<h;y++){
        rsum=gsum=bsum=asum=0;
        for(i=-radius;i<=radius;i++){
            p = (yi + qMin(wm, qMax(i,0))) * 4;
            rsum += pix[p];
            gsum += pix[p+1];
            bsum += pix[p+2];
            asum += pix[p+3];
        }
        for (x=0;x<w;x++){

            r[yi]=dv[rsum];
            g[yi]=dv[gsum];
            b[yi]=dv[bsum];
            a[yi]=dv[asum];

            if(y==0){
                vMIN[x]=qMin(x+radius+1,wm);
                vMAX[x]=qMax(x-radius,0);
            }
            p1 = (yw+vMIN[x])*4;
            p2 = (yw+vMAX[x])*4;

            rsum += pix[p1]		- pix[p2];
            gsum += pix[p1+1]	- pix[p2+1];
            bsum += pix[p1+2]	- pix[p2+2];
            asum += pix[p1+3]	- pix[p2+3];

            yi++;
        }
        yw+=w;
    }

    for (x=0;x<w;x++){
        rsum=gsum=bsum=asum=0;
        yp=-radius*w;
        for(i=-radius;i<=radius;i++){
            yi=qMax(0,yp)+x;
            rsum+=r[yi];
            gsum+=g[yi];
            bsum+=b[yi];
            asum+=a[yi];
            yp+=w;
        }
        yi=x;
        for (y=0;y<h;y++){
            pix[yi*4]		= dv[rsum];
            pix[yi*4 + 1]	= dv[gsum];
            pix[yi*4 + 2]	= dv[bsum];
            pix[yi*4 + 3]	= dv[asum];
            if(x==0){
                vMIN[y]=qMin(y+radius+1,hm)*w;
                vMAX[y]=qMax(y-radius,0)*w;
            }
            p1=x+vMIN[y];
            p2=x+vMAX[y];

            rsum+=r[p1]-r[p2];
            gsum+=g[p1]-g[p2];
            bsum+=b[p1]-b[p2];
            asum+=a[p1]-a[p2];

            yi+=w;
        }
    }

    delete[] r;
    delete[] g;
    delete[] b;
    delete[] a;

    delete[] vMIN;
    delete[] vMAX;
    delete[] dv;
}

void blur(const image &im, qreal radius, qreal sigma)
{
    if(!checkImage(im))
        return;

    qreal *kernel;
    rgba *dest;
    s32 width;
    s32 x, y;
    rgba *scanline, *temp;
    rgba *p, *q;

    if(sigma == 0.0)
        return;

    kernel = 0;

    if(radius > 0)
        width = getBlurKernel((s32)(2*ceil(radius)+1), sigma, &kernel);
    else
    {
        qreal *last_kernel = 0;

        width = getBlurKernel(3, sigma, &kernel);

        while((long)(MaxRGB * kernel[0]) > 0)
        {
            if(last_kernel)
                delete [] last_kernel;

            last_kernel = kernel;
            kernel = 0;

            width = getBlurKernel(width+2, sigma, &kernel);
        }

        if(last_kernel)
        {
            delete [] kernel;
            width -= 2;
            kernel = last_kernel;
        }
    }

    if(width < 3)
    {
        delete [] kernel;
        return;
    }

    dest = new rgba [im.rw * im.rh];

    if(!dest)
    {
        delete [] kernel;
        return;
    }

    scanline = new rgba [im.h];
    temp = new rgba [im.h];

    if(!scanline || !temp)
    {
        if(scanline) delete [] scanline;
        if(temp) delete [] temp;

        delete [] kernel;
        return;
    }

    rgba *bits = (rgba *)im.data;

    for(y = 0;y < im.h;++y)
    {
        p = bits + im.rw*y;
        q = dest + im.rw*y;

        blurScanLine(kernel, width, p, q, im.w);
    }

    for(x = 0;x < im.w;++x)
    {
        for(y = 0;y < im.h;++y)
            scanline[y] = *(dest + im.rw*y + x);
            //scanline[y] = *(bits + im.rw*y + x);

        blurScanLine(kernel, width, scanline, temp, im.h);

        for(y = 0;y < im.h;++y)
            *(dest + im.rw*y + x) = temp[y];

    }

    delete [] scanline;
    delete [] temp;
    delete [] kernel;

    memcpy(im.data, dest, im.rw * im.rh * sizeof(rgba));

    delete [] dest;
}

void equalize(const image &im)
{
    if(!checkImage(im))
        return;

    double_packet high, low, intensity, *map, *histogram;
    short_packet *equalize_map;
    s32 x, y;
    rgba *p, *q;
    long i;
    u8 r, g, b, a;

    histogram = new double_packet [256];
    map = new double_packet [256];
    equalize_map = new short_packet [256];

    if(!histogram || !map || !equalize_map)
    {
        if(histogram)    delete [] histogram;
        if(map)          delete [] map;
        if(equalize_map) delete [] equalize_map;

        return;
    }

    rgba *bits = (rgba *)im.data;

    /*
     *  Form histogram.
     */
    memset(histogram, 0, 256 * sizeof(double_packet));

    for(y = 0;y < im.h;++y)
    {
        p = bits + im.rw * y;

        for(x = 0;x < im.w;++x)
        {
            histogram[p->r].red++;
            histogram[p->g].green++;
            histogram[p->b].blue++;
            histogram[p->a].alpha++;

            p++;
        }
    }
    /*
     Integrate the histogram to get the equalization map.
     */
    memset(&intensity, 0 ,sizeof(double_packet));

    for(i = 0;i < 256;++i)
    {
        intensity.red   += histogram[i].red;
        intensity.green += histogram[i].green;
        intensity.blue  += histogram[i].blue;
        intensity.alpha += histogram[i].alpha;

        map[i] = intensity;
    }

    low=map[0];
    high=map[255];
    memset(equalize_map, 0, 256 * sizeof(short_packet));

    for(i = 0;i < 256;++i)
    {
        if(high.red != low.red)
            equalize_map[i].red=(unsigned short)
                ((65535*(map[i].red-low.red))/(high.red-low.red));
        if(high.green != low.green)
            equalize_map[i].green=(unsigned short)
                ((65535*(map[i].green-low.green))/(high.green-low.green));
        if(high.blue != low.blue)
            equalize_map[i].blue=(unsigned short)
                ((65535*(map[i].blue-low.blue))/(high.blue-low.blue));
        if(high.alpha != low.alpha)
            equalize_map[i].alpha=(unsigned short)
                ((65535*(map[i].alpha-low.alpha))/(high.alpha-low.alpha));
    }

    delete [] histogram;
    delete [] map;

    /*
     Stretch the histogram.
     */
    for(y = 0;y < im.h;++y)
    {
        q = bits + im.rw*y;

        for(x = 0;x < im.w;++x)
        {
            if(low.red != high.red)
                r = (equalize_map[(unsigned short)(q->r)].red/257);
            else
                r = q->r;
            if(low.green != high.green)
                g = (equalize_map[(unsigned short)(q->g)].green/257);
            else
                g = q->g;
            if(low.blue != high.blue)
                b = (equalize_map[(unsigned short)(q->b)].blue/257);
            else
                b = q->b;
            if(low.alpha != high.alpha)
                a = (equalize_map[(unsigned short)(q->a)].alpha/257);
            else
                a = q->a;

            *q = rgba(r, g, b, a);

            q++;
        }
    }

    delete [] equalize_map;
}

struct PointInfo
{
    qreal x, y, z;
};

void shade(const image &im, bool color_shading, qreal azimuth,
             qreal elevation)
{
    if(!checkImage(im))
        return;

    rgba *n = new rgba [im.rw * im.rh];

    if(!n)
        return;

    qreal distance, normal_distance, shade;
    s32 x, y;

    struct PointInfo light, normal;

    rgba *bits;
    rgba *q;

    azimuth = DegreesToRadians(azimuth);
    elevation = DegreesToRadians(elevation);
    light.x = MaxRGB*cos(azimuth)*cos(elevation);
    light.y = MaxRGB*sin(azimuth)*cos(elevation);
    light.z = MaxRGB*sin(elevation);
    normal.z= 2*MaxRGB;  // constant Z of surface normal

    rgba *s0, *s1, *s2;

    for(y = 0;y < im.h;++y)
    {
        bits = (rgba *)im.data + im.rw * (F_MIN(F_MAX(y-1,0),im.h-3));
        q = n + im.rw * y;

        // shade this row of pixels.
        *q++ = (*(bits+im.rw));
        bits++;

        s0 = bits;
        s1 = bits + im.rw;
        s2 = bits + 2*im.rw;

        for(x = 1;x < im.w-1;++x)
        {
            // determine the surface normal and compute shading.
            normal.x = intensityValue(*(s0-1))+intensityValue(*(s1-1))+intensityValue(*(s2-1))-
                (double) intensityValue(*(s0+1))-(double) intensityValue(*(s1+1))-
                (double) intensityValue(*(s2+1));

            normal.y = intensityValue(*(s2-1))+intensityValue(*s2)+intensityValue(*(s2+1))-
                (double) intensityValue(*(s0-1))-(double) intensityValue(*s0)-
                (double) intensityValue(*(s0+1));

            if(normal.x == 0 && normal.y == 0)
                shade = light.z;
            else
            {
                shade = 0.0;
                distance = normal.x*light.x+normal.y*light.y+normal.z*light.z;

                if(distance > 0.0)
                {
                    normal_distance = normal.x*normal.x+normal.y*normal.y+normal.z*normal.z;

                    if(fabs(normal_distance) > 0.0000001)
                        shade=distance/sqrt(normal_distance);
                }
            }

            if(!color_shading)
            {
                *q = rgba((u8)(shade),
                           (u8)(shade),
                           (u8)(shade),
                           s1->a);
            }
            else
            {
                *q = rgba((u8)((shade * s1->r)/(MaxRGB+1)),
                           (u8)((shade * s1->g)/(MaxRGB+1)),
                           (u8)((shade * s1->b)/(MaxRGB+1)),
                           s1->a);
            }

            ++s0;
            ++s1;
            ++s2;
            q++;
        }

        *q++ = (*s1);
    }

    memcpy(im.data, n, im.rw * im.rh * sizeof(rgba));

    delete [] n;
}

void edge(image &im, qreal radius)
{
    if(!checkImage(im))
        return;

    qreal *kernel;
    int width;
    register long i;
    rgba *dest = 0;

    width = getOptimalKernelWidth(radius, 0.5);

    const int W = width*width;

    if(im.w < width || im.h < width)
        return;

    kernel = new qreal [W];

    if(!kernel)
        return;

    for(i = 0;i < W;i++)
        kernel[i] = -1.0;

    kernel[i/2] = W-1.0;

    if(!convolveImage(&im, &dest, width, kernel))
    {
        delete [] kernel;

        if(dest)
            delete [] dest;

        return;
    }

    delete [] kernel;

    memcpy(im.data, dest, im.rw * im.rh * sizeof(rgba));

    delete [] dest;
}

void emboss(image &im, qreal radius, qreal sigma)
{
    if(!checkImage(im))
        return;

    qreal alpha, *kernel;
    int j, width;
    register long i, u, v;
    rgba *dest = 0;

    if(sigma == 0.0)
        return;

    width = getOptimalKernelWidth(radius, sigma);

    if(im.w < width || im.h < width)
        return;

    kernel = new qreal [width*width];

    if(!kernel)
        return;

    i = 0;
    j = width/2;

    const qreal S = sigma * sigma;

    for(v = (-width/2);v <= (width/2);v++)
    {
        for(u=(-width/2); u <= (width/2); u++)
        {
            alpha = exp(-((double) u*u+v*v)/(2.0*S));

            kernel[i] = ((u < 0) || (v < 0) ? -8.0 : 8.0)*alpha/(2.0*MagickPI*S);

            if (u == j)
                kernel[i]=0.0;

            i++;
        }

        j--;
    }

    if(!convolveImage(&im, &dest, width, kernel))
    {
        delete [] kernel;
        return;
    }

    delete [] kernel;

    fmt_filters::image mm((u8 *)dest, im.w, im.h, im.rw, im.rh);

    equalize(mm);

    memcpy(im.data, dest, im.rw * im.rh * sizeof(rgba));

    delete [] dest;
}

void sharpen(image &im, qreal radius, qreal sigma)
{
    if(!checkImage(im))
        return;

    qreal alpha, normalize, *kernel;
    int width;
    register long i, u, v;
    rgba *dest = 0;

    if(sigma == 0.0)
        sigma = 0.01;

    width = getOptimalKernelWidth(radius, sigma);

    if(im.w < width)
        return;

    kernel = new qreal [width*width];

    if(!kernel)
        return;

    i = 0;
    normalize = 0.0;
    const qreal S = sigma * sigma;
    const int w2 = width / 2;

    for(v = -w2; v <= w2; v++)
    {
        for(u = -w2; u <= w2; u++)
        {
            alpha = exp(-((double) u*u+v*v)/(2.0*S));
            kernel[i] = alpha/(2.0*MagickPI*S);
            normalize += kernel[i];

            i++;
        }
    }

    kernel[i/2] = (-2.0)*normalize;

    if(!convolveImage(&im, &dest, width, kernel))
    {
        delete [] kernel;

        if(dest)
            delete [] dest;

        return;
    }

    delete [] kernel;

    memcpy(im.data, dest, im.rw * im.rh * sizeof(rgba));

    delete [] dest;
}

void oil(const image &im, qreal radius)
{
    if(!checkImage(im))
        return;

    unsigned long count;
    unsigned long histogram[256];
    unsigned int k;
    int width;
    int x, y, mx, my, sx, sy;
    int mcx, mcy;
    rgba *s = 0, *q;

    scaleDown(radius, 1.0, 5.0);

    rgba *n = new rgba [im.rw * im.rh];

    if(!n)
        return;

    memcpy(n, im.data, im.rw * im.rh * sizeof(rgba));

    width = getOptimalKernelWidth(radius, 0.5);

    if(im.w < width)
    {
        delete [] n;
        return;
    }

    rgba *bits = (rgba *)im.data;

    for(y = 0;y < im.h;++y)
    {
        sy = y-(width/2);
        q = n + im.rw*y;

        for(x = 0;x < im.w;++x)
        {
            count = 0;
            memset(histogram, 0, 256 * sizeof(unsigned long));
            sy = y-(width/2);

            for(mcy = 0;mcy < width;++mcy,++sy)
            {
                my = sy < 0 ? 0 : sy > im.h-1 ? im.h-1 : sy;
                sx = x+(-width/2);

                for(mcx = 0; mcx < width;++mcx,++sx)
                {
                    mx = sx < 0 ? 0 : sx > im.w-1 ? im.w-1 : sx;

                    k = intensityValue(*(bits + my*im.rw + mx));

                    if(k > 255) k = 255;

                    histogram[k]++;

                    if(histogram[k] > count)
                    {
                        count = histogram[k];
                        s = bits + my*im.rw + mx;
                    }
                }
            }

            *q++ = (*s);
        }
    }

    memcpy(im.data, n, im.rw * im.rh * sizeof(rgba));

    delete [] n;
}

void redeye(const image &im, const int w, const int h, const int x, const int y, int th)
{
    const qreal RED_FACTOR = 0.5133333;
    const qreal GREEN_FACTOR = 1;
    const qreal BLUE_FACTOR = 0.1933333;

    if(!checkImage(im))
        return;

    scaleDown(th, 0, 255);

    int y1, x1;
    int adjusted_red, adjusted_green, adjusted_blue;

    rgba *src = (rgba *)im.data, *s;

    for(y1 = y;y1 < y+h;++y1)
    {
        s = src + im.w*y1 + x;

        for(x1 = x;x1 < x+w;x1++)
        {
            adjusted_red = int(s->r * RED_FACTOR);
            adjusted_green = int(s->g * GREEN_FACTOR);
            adjusted_blue = int(s->b * BLUE_FACTOR);

            if(adjusted_red >= adjusted_green - th && adjusted_red >= adjusted_blue - th)
                s->r = (int)(((double)(adjusted_green + adjusted_blue)) / (2.0  * RED_FACTOR));

            s++;
        }
    }
}


/*************************************************************************/

/*
 *
 *  Helper functions
 *
 */

/*************************************************************************/

static bool convolveImage(image *image, rgba **dest, const unsigned int order,
                                 const qreal *kernel)
{
    long width;
    qreal red, green, blue;
    u8 alpha;
    qreal normalize, *normal_kernel;
    register const qreal *k;
    register rgba *q;
    int x, y, mx, my, sx, sy;
    long i;
    int mcx, mcy;

    width = order;

    if((width % 2) == 0)
        return false;

    const int W = width*width;

    normal_kernel = new qreal [W];

    if(!normal_kernel)
        return false;

    *dest = new rgba [image->rw * image->rh];

    if(!*dest)
    {
        delete [] normal_kernel;
        return false;
    }

    normalize = 0.0;

    for(i = 0;i < W;i++)
        normalize += kernel[i];

    if(fabs(normalize) <= MagickEpsilon)
        normalize = 1.0;

    normalize=1.0/normalize;

    for(i = 0;i < W;i++)
        normal_kernel[i] = normalize*kernel[i];

    rgba *bits = (rgba *)image->data;

    for(y = 0;y < image->h;++y)
    {
        sy = y-(width/2);
        q = *dest + image->rw * y;

        for(x = 0;x < image->w;++x)
        {
            k = normal_kernel;
            red = green = blue = alpha = 0;
            sy = y-(width/2);
            alpha = (bits + image->rw*y+x)->a;

            for(mcy=0; mcy < width; ++mcy, ++sy)
            {
                my = sy < 0 ? 0 : sy > image->h-1 ? image->h-1 : sy;
                sx = x+(-width/2);

                for(mcx=0; mcx < width; ++mcx, ++sx)
                {
                    mx = sx < 0 ? 0 : sx > image->w-1 ? image->w-1 : sx;
                    red +=   (*k) * ((bits + image->rw*my+mx)->r*257);
                    green += (*k) * ((bits + image->rw*my+mx)->g*257);
                    blue +=  (*k) * ((bits + image->rw*my+mx)->b*257);
//                    alpha += (*k) * ((bits + image->rw*my+mx)->a*257);

                    ++k;
                }
            }

            red = red < 0 ? 0 : red > 65535 ? 65535 : red+0.5;
            green = green < 0 ? 0 : green > 65535 ? 65535 : green+0.5;
            blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue+0.5;
//            alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha+0.5;

            *q++ = rgba((unsigned char)(red/257UL),
                         (unsigned char)(green/257UL),
                         (unsigned char)(blue/257UL),
                         alpha);
        }
    }

    delete [] normal_kernel;

    return true;
}

static void rgb2hsv(const rgb &rgb, s32 *h, s32 *s, s32 *v)
{
    if(!h || !s || !v)
        return;

    s32 r = rgb.r;
    s32 g = rgb.g;
    s32 b = rgb.b;

    u32 max = r;
    s32 whatmax = 0;                // r=>0, g=>1, b=>2

    if((u32)g > max)
    {
        max = g;
        whatmax = 1;
    }

    if((u32)b > max)
    {
        max = b;
        whatmax = 2;
    }

    u32 min = r;               // find minimum value
    if((u32)g < min) min = g;
    if((u32)b < min) min = b;

    s32 delta = max-min;
    *v = max;                   // calc value
    *s = max ? (510*delta+max)/(2*max) : 0;

    if(*s == 0)
    {
        *h = -1;                // undefined hue
    }
    else
    {
        switch(whatmax)
        {
            case 0:             // red is max component
                if(g >= b)
                    *h = (120*(g-b)+delta)/(2*delta);
                else
                    *h = (120*(g-b+delta)+delta)/(2*delta) + 300;
            break;

            case 1:             // green is max component
                if(b > r)
                    *h = 120 + (120*(b-r)+delta)/(2*delta);
                else
                    *h = 60 + (120*(b-r+delta)+delta)/(2*delta);
            break;

            case 2:             // blue is max component
                if(r > g)
                    *h = 240 + (120*(r-g)+delta)/(2*delta);
                else
                    *h = 180 + (120*(r-g+delta)+delta)/(2*delta);
            break;
        }
    }
}

static void hsv2rgb(s32 h, s32 s, s32 v, rgb *rgb) {
    if(h < -1 || (u32)s > 255 || (u32)v > 255 || !rgb)
        return;

    s32 r = v, g = v, b = v;

    if(s == 0 || h == -1)
    {
        // Ignore
    }
    else
    {                    // chromatic case
        if((u32)h >= 360)
            h %= 360;

        u32 f = h%60;
        h /= 60;
        u32 p = (u32)(2*v*(255-s)+255)/510;
        u32 q, t;

        if(h&1)
        {
            q = (u32)(2*v*(15300-s*f)+15300)/30600;

            switch(h)
            {
                case 1: r=(s32)q; g=(s32)v, b=(s32)p; break;
                case 3: r=(s32)p; g=(s32)q, b=(s32)v; break;
                case 5: r=(s32)v; g=(s32)p, b=(s32)q; break;
            }
        }
        else
        {
            t = (u32)(2*v*(15300-(s*(60-f)))+15300)/30600;

            switch(h)
            {
                case 0: r=(s32)v; g=(s32)t, b=(s32)p; break;
                case 2: r=(s32)p; g=(s32)v, b=(s32)t; break;
                case 4: r=(s32)t; g=(s32)p, b=(s32)v; break;
            }
        }
    }

    rgb->r = r;
    rgb->g = g;
    rgb->b = b;
}

static rgba interpolateColor(const image &im, qreal x_offset, qreal y_offset, const rgba &background)
{
    qreal alpha, beta;
    rgba p, q, r, s;
    s32 x, y;
    rgba *bits = (rgba *)im.data;

    if(!checkImage(im))
        return background;

    x = (s32)x_offset;
    y = (s32)y_offset;

    if((x < -1) || (x >= im.w) || (y < -1) || (y >= im.h))
        return background;

    if((x >= 0) && (y >= 0) && (x < (im.w-1)) && (y < (im.h-1)))
    {
        rgba *t = bits + y * im.rw;

        p = t[x];
        q = t[x+1];
        r = t[x+im.rw];
        s = t[x+im.rw+1];
    }
    else
    {
        rgba *t = bits + y * im.rw;

        p = background;

        if((x >= 0) && (y >= 0))
            p = t[x];

        q = background;

        if(((x+1) < im.w) && (y >= 0))
            q = t[x+1];

        r = background;

        if((x >= 0) && ((y+1) < im.h))
        {
            t = bits + (y+1) * im.rw;
            r = t[x+im.rw];
        }

        s = background;

        if(((x+1) < im.w) && ((y+1) < im.h))
        {
            t = bits + (y+1) * im.rw;
            s = t[x+im.rw+1];
        }
    }

    x_offset -= floor(x_offset);
    y_offset -= floor(y_offset);
    alpha = 1.0-x_offset;
    beta = 1.0-y_offset;

    rgba _r;

    _r.r = (u8)(beta * (alpha*p.r + x_offset*q.r) + y_offset * (alpha*r.r + x_offset*s.r));
    _r.g = (u8)(beta * (alpha*p.g + x_offset*q.g) + y_offset * (alpha*r.g + x_offset*s.g));
    _r.b = (u8)(beta * (alpha*p.b + x_offset*q.b) + y_offset * (alpha*r.b + x_offset*s.b));
    _r.a = (u8)(beta * (alpha*p.a + x_offset*q.a) + y_offset * (alpha*r.a + x_offset*s.a));

    return _r;
}

static u32 generateNoise(u32 pixel, NoiseType noise_type)
{
#define NoiseEpsilon  1.0e-5
#define NoiseMask  0x7fff
#define SigmaUniform  4.0
#define SigmaGaussian  4.0
#define SigmaImpulse  0.10
#define SigmaLaplacian 10.0
#define SigmaMultiplicativeGaussian  0.5
#define SigmaPoisson  0.05
#define TauGaussian  20.0

    qreal alpha, beta, sigma, value;
    alpha=(double) (rand() & NoiseMask)/NoiseMask;
    if (alpha == 0.0)
        alpha=1.0;
    switch(noise_type){
    case UniformNoise:
    default:
        {
            value=(double) pixel+SigmaUniform*(alpha-0.5);
            break;
        }
    case GaussianNoise:
        {
            qreal tau;

            beta=(double) (rand() & NoiseMask)/NoiseMask;
            sigma=sqrt(-2.0*log(alpha))*cos(2.0*M_PI*beta);
            tau=sqrt(-2.0*log(alpha))*sin(2.0*M_PI*beta);
            value=(double) pixel+
                (sqrt((double) pixel)*SigmaGaussian*sigma)+(TauGaussian*tau);
            break;
        }
    case MultiplicativeGaussianNoise:
        {
            if (alpha <= NoiseEpsilon)
                sigma=MaxRGB;
            else
                sigma=sqrt(-2.0*log(alpha));
            beta=(rand() & NoiseMask)/NoiseMask;
            value=(double) pixel+
                pixel*SigmaMultiplicativeGaussian*sigma*cos(2.0*M_PI*beta);
            break;
        }
    case ImpulseNoise:
        {
            if (alpha < (SigmaImpulse/2.0))
                value=0;
            else
                if (alpha >= (1.0-(SigmaImpulse/2.0)))
                    value=MaxRGB;
                else
                    value=pixel;
            break;
        }
    case LaplacianNoise:
        {
            if (alpha <= 0.5)
            {
                if (alpha <= NoiseEpsilon)
                    value=(double) pixel-MaxRGB;
                else
                    value=(double) pixel+SigmaLaplacian*log(2.0*alpha);
                break;
            }
            beta=1.0-alpha;
            if (beta <= (0.5*NoiseEpsilon))
                value=(double) pixel+MaxRGB;
            else
                value=(double) pixel-SigmaLaplacian*log(2.0*beta);
            break;
        }
    case PoissonNoise:
        {
            register s32
                i;

            for (i=0; alpha > exp(-SigmaPoisson*pixel); i++)
            {
                beta=(double) (rand() & NoiseMask)/NoiseMask;
                alpha=alpha*beta;
            }
            value=i/SigmaPoisson;
            break;
        }
    }

    if(value < 0.0)
        return 0;
    else if(value > MaxRGB)
        return MaxRGB;
    else
        return ((u32) (value+0.5));
}

static inline u32 intensityValue(s32 r, s32 g, s32 b)
{
    return ((u32)((0.299*r + 0.587*g + 0.1140000000000001*b)));
}

static inline u32 intensityValue(const rgba &rr)
{
    return ((u32)((0.299*rr.r + 0.587*rr.g + 0.1140000000000001*rr.b)));
}

template<class T>
static inline void scaleDown(T &val, T min, T max)
{
    if(val < min)
        val = min;
    else if(val > max)
        val = max;
}

static void blurScanLine(qreal *kernel, s32 width,
                         rgba *src, rgba *dest, s32 columns) {
    register qreal *p;
    rgba *q;
    register s32 x;
    register long i;
    qreal red, green, blue, alpha;
    qreal scale = 0.0;

    if(width > columns)
    {
        for(x = 0;x < columns;++x)
        {
            scale = 0.0;
            red = blue = green = alpha = 0.0;
            p = kernel;
            q = src;

            for(i = 0;i < columns;++i)
            {
                if((i >= (x-width/2)) && (i <= (x+width/2)))
                {
                    red += (*p)*(q->r * 257);
                    green += (*p)*(q->g*257);
                    blue += (*p)*(q->b*257);
                    alpha += (*p)*(q->a*257);
                }

                if(((i+width/2-x) >= 0) && ((i+width/2-x) < width))
                    scale += kernel[i+width/2-x];

                p++;
                q++;
            }

            scale = 1.0/scale;
            red = scale*(red+0.5);
            green = scale*(green+0.5);
            blue = scale*(blue+0.5);
            alpha = scale*(alpha+0.5);

            red = red < 0 ? 0 : red > 65535 ? 65535 : red;
            green = green < 0 ? 0 : green > 65535 ? 65535 : green;
            blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue;
            alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha;

            dest[x] = rgba((u8)(red/257UL),
                            (u8)(green/257UL),
                            (u8)(blue/257UL),
                            (u8)(alpha/257UL));
        }

        return;
    }

    for(x = 0;x < width/2;++x)
    {
        scale = 0.0;
        red = blue = green = alpha = 0.0;
        p = kernel+width/2-x;
        q = src;

        for(i = width/2-x;i < width;++i)
        {
            red += (*p)*(q->r*257);
            green += (*p)*(q->g*257);
            blue += (*p)*(q->b*257);
            alpha += (*p)*(q->a*257);
            scale += (*p);
            p++;
            q++;
        }

        scale=1.0/scale;

        red = scale*(red+0.5);
        green = scale*(green+0.5);
        blue = scale*(blue+0.5);
        alpha = scale*(alpha+0.5);

        red = red < 0 ? 0 : red > 65535 ? 65535 : red;
        green = green < 0 ? 0 : green > 65535 ? 65535 : green;
        blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue;
        alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha;

        dest[x] = rgba((u8)(red/257UL),
                        (u8)(green/257UL),
                        (u8)(blue/257UL),
                        (u8)(alpha/257UL));
    }

    for(;x < columns-width/2;++x)
    {
        red = blue = green = alpha = 0.0;
        p = kernel;
        q = src+(x-width/2);

        for(i = 0;i < (long)width;++i)
        {
            red += (*p)*(q->r*257);
            green += (*p)*(q->g*257);
            blue += (*p)*(q->b*257);
            alpha += (*p)*(q->a*257);
            p++;
            q++;
        }

        red = scale*(red+0.5);
        green = scale*(green+0.5);
        blue = scale*(blue+0.5);
        alpha = scale*(alpha+0.5);

        red = red < 0 ? 0 : red > 65535 ? 65535 : red;
        green = green < 0 ? 0 : green > 65535 ? 65535 : green;
        blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue;
        alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha;

        dest[x] = rgba((u8)(red/257UL),
                        (u8)(green/257UL),
                        (u8)(blue/257UL),
                        (u8)(alpha/257UL));
    }

    for(;x < columns;++x)
    {
        red = blue = green = alpha = 0.0;
        scale=0;
        p = kernel;
        q = src+(x-width/2);

        for(i = 0;i < columns-x+width/2;++i)
        {
            red += (*p)*(q->r*257);
            green += (*p)*(q->g*257);
            blue += (*p)*(q->b*257);
            alpha += (*p)*(q->a*257);
            scale += (*p);
            p++;
            q++;
        }

        scale=1.0/scale;
        red = scale*(red+0.5);
        green = scale*(green+0.5);
        blue = scale*(blue+0.5);
        alpha = scale*(alpha+0.5);

        red = red < 0 ? 0 : red > 65535 ? 65535 : red;
        green = green < 0 ? 0 : green > 65535 ? 65535 : green;
        blue = blue < 0 ? 0 : blue > 65535 ? 65535 : blue;
        alpha = alpha < 0 ? 0 : alpha > 65535 ? 65535 : alpha;

        dest[x] = rgba((u8)(red/257UL),
                        (u8)(green/257UL),
                        (u8)(blue/257UL),
                        (u8)(alpha/257UL));
    }
}

static s32 getBlurKernel(s32 width, qreal sigma, qreal **kernel)
{

#define KernelRank  3
#define KernelRankQ 18.0

    qreal alpha, normalize;
    register long i;
    s32 bias;

    if(sigma == 0.0)
        return 0;

    if(width == 0)
        width = 3;

    *kernel = new qreal [width];

    if(!*kernel)
        return 0;

    memset(*kernel, 0, width * sizeof(double));
    bias = KernelRank * width/2;

    for(i = (-bias);i <= bias; i++)
    {
        alpha = exp(-((double) i*i)/(KernelRankQ*sigma*sigma));
        (*kernel)[(i+bias)/KernelRank] += alpha/(MagickSQ2PI*sigma);
    }

    normalize = 0;

    for(i = 0;i < width;i++)
        normalize += (*kernel)[i];

    for(i = 0;i < width;i++)
        (*kernel)[i] /= normalize;

    return width;

#undef KernelRankQ
#undef KernelRank

}

void qblurMono(qreal *data,
               const int &w,
               const int &h,
               const qreal &fRadius) {
    if(fRadius < 0.01) return;
    qreal divF = fRadius + fRadius + 1.;
    qreal divFInv = 1./divF;
    int iRadius = ceil(fRadius);
    int nPoints = iRadius + iRadius + 1;
    qreal fracInf = 1. - iRadius + fRadius;
    qreal fracInfInv = 1. - fracInf;

    qreal *valLine = new qreal[nPoints];

    int wm=w-1;
    int hm=h-1;
    int wh=w*h;
    qreal *v = new qreal[wh];
    qreal vsum;
    int x,y,i,p,p1,yp,yi,yw;
    int *vMIN = new int[qMax(w,h)];
    int *vMAX = new int[qMax(w,h)];


    yw=yi=0;

    for(y = 0; y < h; y++) {
        p = yi;
        valLine[0] = data[p];
        vsum = data[p]*fracInf;

        for(i = 1 - iRadius; i < iRadius; i++) {
            p = yi + qMin(wm, qMax(i,0));
            valLine[i + iRadius] = data[p];
            vsum += data[p];
        }

        p = yi + qMin(wm, iRadius);
        valLine[iRadius + iRadius] = data[p];
        vsum += data[p]*fracInf;

        for(x = 0; x < w; x++) {

            v[yi] = vsum*divFInv;

            if(y == 0) {
                vMIN[x] = qMin(x+iRadius+1, wm);
                vMAX[x] = qMax(x-iRadius, 0);
            }
            p1 = yw + vMIN[x];

            vsum -= valLine[0]*fracInf;

            vsum -= valLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                valLine[i] = valLine[i + 1];
            }

            vsum += valLine[nPoints - 1]*fracInfInv;

            valLine[nPoints - 1] = data[p1];

            vsum += data[p1]*fracInf;

            yi++;
        }
        yw+=w;
    }

    for(x = 0; x < w; x++) {
        yp=-iRadius*w;

        yi=qMax(0,yp)+x;
        valLine[0] = v[yi];
        vsum = v[yi]*fracInf;
        yp+=w;

        for(i = 1 - iRadius; i < iRadius; i++) {
            yi = qMax(0, yp) + x;
            valLine[i + iRadius] = v[yi];
            vsum += v[yi];
            yp+=w;
        }

        yi = qMax(0, yp)+x;
        valLine[iRadius + iRadius] = v[yi];
        vsum += v[yi]*fracInf;
        yp+=w;


        yi=x;
        for(y = 0; y < h; y++) {
            data[yi]	= vsum*divFInv;

            if(x==0) {
                vMIN[y] = qMin(y+iRadius+1,hm)*w;
                vMAX[y] = qMax(y-iRadius,0)*w;
            }
            p1=x+vMIN[y];

            vsum -= valLine[0]*fracInf;

            vsum -= valLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                valLine[i] = valLine[i + 1];
            }

            vsum += valLine[nPoints - 1]*fracInfInv;

            valLine[nPoints - 1] = v[p1];

            vsum += v[p1]*fracInf;

            yi+=w;
        }
    }

    delete[] valLine;

    delete[] v;

    delete[] vMIN;
    delete[] vMAX;
}


void qspredMono(qreal *data,
               const int &w,
               const int &h,
               const qreal &fRadius) {
    if(fRadius < 0.01) return;
    qreal divF = fRadius + fRadius + 1.;
    qreal divFInv = 1./divF;
    int iRadius = ceil(fRadius);
    int nPoints = iRadius + iRadius + 1;
    qreal fracInf = 1. - iRadius + fRadius;
    qreal fracInfInv = 1. - fracInf;

    qreal *valLine = new qreal[nPoints];

    int wm=w-1;
    int hm=h-1;
    int wh=w*h;
    qreal *v = new qreal[wh];
    qreal vsum;
    int x,y,i,p,p1,yp,yi,yw;
    int *vMIN = new int[qMax(w,h)];
    int *vMAX = new int[qMax(w,h)];


    yw=yi=0;

    for(y = 0; y < h; y++) {
        p = yi;
        valLine[0] = data[p];
        vsum = data[p]*fracInf;

        for(i = 1 - iRadius; i < iRadius; i++) {
            p = yi + qMin(wm, qMax(i,0));
            valLine[i + iRadius] = data[p];
            vsum += data[p];
        }

        p = yi + qMin(wm, iRadius);
        valLine[iRadius + iRadius] = data[p];
        vsum += data[p]*fracInf;

        for(x = 0; x < w; x++) {

            v[yi] = vsum*divFInv;

            if(y == 0) {
                vMIN[x] = qMin(x+iRadius+1, wm);
                vMAX[x] = qMax(x-iRadius, 0);
            }
            p1 = yw + vMIN[x];

            vsum -= valLine[0]*fracInf;

            vsum -= valLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                valLine[i] = valLine[i + 1];
            }

            vsum += valLine[nPoints - 1]*fracInfInv;

            valLine[nPoints - 1] = data[p1];

            vsum += data[p1]*fracInf;

            yi++;
        }
        yw+=w;
    }

    for(x = 0; x < w; x++) {
        yp=-iRadius*w;

        yi=qMax(0,yp)+x;
        valLine[0] = v[yi];
        vsum = v[yi]*fracInf;
        yp+=w;

        for(i = 1 - iRadius; i < iRadius; i++) {
            yi = qMax(0, yp) + x;
            valLine[i + iRadius] = v[yi];
            vsum += v[yi];
            yp+=w;
        }

        yi = qMax(0, yp)+x;
        valLine[iRadius + iRadius] = v[yi];
        vsum += v[yi]*fracInf;
        yp+=w;


        yi=x;
        for(y = 0; y < h; y++) {
            data[yi] = qMax(data[yi], vsum*divFInv);

            if(x==0) {
                vMIN[y] = qMin(y+iRadius+1,hm)*w;
                vMAX[y] = qMax(y-iRadius,0)*w;
            }
            p1=x+vMIN[y];

            vsum -= valLine[0]*fracInf;

            vsum -= valLine[1]*fracInfInv;

            for(i = 0; i < nPoints - 1; i++) {
                valLine[i] = valLine[i + 1];
            }

            vsum += valLine[nPoints - 1]*fracInfInv;

            valLine[nPoints - 1] = v[p1];

            vsum += v[p1]*fracInf;

            yi+=w;
        }
    }

    delete[] valLine;

    delete[] v;

    delete[] vMIN;
    delete[] vMAX;
}

static void hull(const s32 x_offset, const s32 y_offset, const s32 polarity, const s32 columns,
                        const s32 rows, u8 *f, u8 *g)
{
    s32 x, y;

    u8 *p, *q, *r, *s;
    u32 v;

    if(f == 0 || g == 0)
        return;

    p = f+(columns+2);
    q = g+(columns+2);
    r = p+(y_offset*(columns+2)+x_offset);

    for(y = 0;y < rows;y++)
    {
        p++;
        q++;
        r++;
        if(polarity > 0)
            for(x = 0;x < columns;x++)
            {
                v=(*p);
                if (*r > v)
                    v++;
                *q=v > 255 ? 255 : v;
                p++;
                q++;
                r++;
            }
        else
            for(x = 0;x < columns;x++)
            {
                v=(*p);
                if (v > (u32) (*r+1))
                    v--;
                *q=v;
                p++;
                q++;
                r++;
            }
        p++;
        q++;
        r++;
    }

    p = f+(columns+2);
    q = g+(columns+2);
    r = q+(y_offset*(columns+2)+x_offset);
    s = q-(y_offset*(columns+2)+x_offset);

    for(y = 0;y < rows;y++)
    {
        p++;
        q++;
        r++;
        s++;

        if(polarity > 0)
            for(x=0; x < (s32) columns; x++)
            {
                v=(*q);
                if (((u32) (*s+1) > v) && (*r > v))
                    v++;
                *p=v > 255 ? 255 : v;
                p++;
                q++;
                r++;
                s++;
            }
        else
            for (x=0; x < columns; x++)
            {
                v=(*q);
                if (((u32) (*s+1) < v) && (*r < v))
                    v--;
                *p=v;
                p++;
                q++;
                r++;
                s++;
            }

        p++;
        q++;
        r++;
        s++;
    }
}

static int getOptimalKernelWidth(qreal radius, qreal sigma)
{
    qreal normalize, value;
    long width;
    register long u;

    if(sigma == 0.0)
        sigma = 0.01;

    if(radius > 0.0)
        return((int)(2.0*ceil(radius)+1.0));

    const qreal S = sigma * sigma;

    for(width = 5;;)
    {
        normalize = 0.0;

        for(u = (-width/2);u <= (width/2);u++)
            normalize+=exp(-((double) u*u)/(2.0*S))/(MagickSQ2PI*sigma);

        u = width/2;
        value = exp(-((double) u*u)/(2.0*S))/(MagickSQ2PI*sigma)/normalize;

        if((long)(65535*value) <= 0)
            break;

        width+=2;
    }

    return ((int)width-2);
}

} // namespace
