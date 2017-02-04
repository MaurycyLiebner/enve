/*
 * Copyright (c) 2005 Dmitry Baryshev <ksquirrel.iv@gmail.com>
 */

/*
 * All methods (except redeye) in this namespace are ported from KDE 3.2.3.
 * All of them are copyrighted by their authors. See fmt_filters_README for more.
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

#ifndef FMT_FILTERS_H
#define FMT_FILTERS_H

//////////////////////////////////////////
//                                      //
//  Pass the image through some filter  //
//                                      //
//////////////////////////////////////////


namespace fmt_filters
{
    typedef char            s8;
    typedef unsigned char   u8;

    typedef short           s16;
    typedef unsigned short  u16;

    typedef int             s32;
    typedef unsigned int    u32;

    struct image
    {
        image() : data(0), w(0), h(0), rw(0), rh(0)
        {}

        image(unsigned char *d, int _w, int _h) : data(d), w(_w), h(_h), rw(_w), rh(_h)
        {}

        image(unsigned char *d, int _w, int _h, int _rw, int _rh) : data(d), w(_w), h(_h), rw(_rw), rh(_rh)
        {}

        unsigned char *data;
        int w;
        int h;
            int rw;
            int rh;
    };

    struct rgb
    {
        rgb() : r(0), g(0), b(0)
        {}

        rgb(int _r, int _g, int _b) : r(_r), g(_g), b(_b)
        {}

        unsigned char r;
        unsigned char g;
        unsigned char b;
    };

        struct rgba
        {
            rgba(int r1, int g1, int b1, int a1) : r(r1), g(g1), b(b1), a(a1)
            {}

            rgba() : r(0), g(0), b(0), a(0)
            {}

            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };

        enum NoiseType
        {
                UniformNoise = 0,            // Uniform distribution
                GaussianNoise,               // Gaussian distribution
                MultiplicativeGaussianNoise, // Multiplicative Gaussian distribution
                ImpulseNoise,                // Impulse distribution
                LaplacianNoise,              // Laplacian distribution
                PoissonNoise                 // Poisson distribution
        };


    bool checkImage(const image &im);

    // colorize the image, which is w x h, left alpha channel unchanged.
    //
    // it just adds to each pixel in the image
    // aproproriate value.
    void colorize(const image &im, int red, int green, int blue);

    // change brightness of the image
    void brightness(const image &im, int bn);

    // change gamma
    // gamma should be  0.0 <= L <= 6.0
    //
    // it is no problem to set L to 8.0 or 9.0, but the resulting
    // image won't have much difference from 6.0
    void gamma(const image &im, double L);

    // change contrast with Photoshop-like method
    // contrast should be  -255 <= contrast <= 255
    void contrast(const image &im, int contrast);

    enum swapRGBtype { GBR = 0, BRG = 1 };

    // negative
    void negative(const image &im);

    // swap RGB values
    void swapRGB(const image &im, int type);

        //
        // All the following filters are ported from KDE's
        // KImageEffect. See kdelibs/kdefx/kimageeffect.cpp
        // for more.
        //

    // blend
        // opacity = [0.0; 1.0]
    void blend(const image &im, const rgb &rgb, float opacity);
    void replaceColor(const image &im,
                      const int &rInt,
                      const int &gInt,
                      const int &bInt);

        // val = [0.0; 1.0]
        void fade(const image &im, const rgb &rgb, float val);

        void gray(const image &im);

        // desat = [0.0; 1.0]
        void desaturate(const image &im, float desat);

        // threshold = [0; 255]
        void threshold(const image &im, unsigned int threshold);

        // factor = [0.0; 50.0]
        void solarize(const image &im, double factor);

        // amount = [1; 10]
        void spread(const image &im, unsigned int amount);

        // degrees = [-720.0; 720.0]
        void swirl(const image &im, double degrees, const rgba &background);

        void noise(const image &im, NoiseType noise_type);

        void flatten(const image &im, const rgb &ca, const rgb &cb);

        // azimuth = [0.0; 90.0], elevation = [0.0; 90.0]
        void shade(const image &im, bool color, double azimuth, double elevation);

        void equalize(const image &im);

        // radius = [0.01; 90.0], sigma = [0.01; 50.0]
        void fast_blur(const image &im, int radius);
        void blur(const image &im, double radius, double sigma);

        void despeckle(const image &im);

        // factor = [0; 100]
        void implode(const image &im, double factor, const rgba &background);

        // radius = [0.01; 30.0]
        void edge(image &im, double radius);

        // radius = [0.01; 99.9], sigma = [0.01; 99.9]
        void emboss(image &im, double radius, double sigma);

        // radius = [0.01; 99.9], sigma = [0.01; 30.0]
        void sharpen(image &im, double radius, double sigma);

        // radius = [1.0; 5.0]
        void oil(const image &im, double radius);

        // red-eye removal.
        // th = [0; 255]
        void redeye(const image &im, const int w, const int h,
                            const int x, const int y,
                            int th);

        // Do we need some color definitions ?
        static const rgba white       = rgba(255, 255, 255, 255);
        static const rgba black       = rgba(0,   0,   0,   255);
        static const rgba red         = rgba(255, 0,   0,   255);
        static const rgba green       = rgba(0,   255, 0,   255);
        static const rgba blue        = rgba(0,   0,   255, 255);
        static const rgba cyan        = rgba(0,   255, 255, 255);
        static const rgba magenta     = rgba(255, 0,   255, 255);
        static const rgba yellow      = rgba(255, 255, 0,   255);
        static const rgba mediumgray  = rgba(128, 128, 128, 255);
        static const rgba lightgray   = rgba(160, 160, 164, 255);
        static const rgba normalgray  = rgba(192, 192, 192, 255);
        static const rgba darkred     = rgba(128, 0,   0,   255);
        static const rgba darkgreen   = rgba(0,   128, 0,   255);
        static const rgba darkblue    = rgba(0,   0,   128, 255);
        static const rgba darkcyan    = rgba(0,   128, 128, 255);
        static const rgba darkmagenta = rgba(128, 0,   128, 255);
        static const rgba darkyellow  = rgba(128, 128, 0,   255);
}

#endif
