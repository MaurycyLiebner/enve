
#ifndef RASTEREFFECTS_H
#define RASTEREFFECTS_H

#include <QtCore>
class SkBitmap;

namespace RasterEffects {
    typedef char            s8;
    typedef unsigned char   u8;

    typedef short           s16;
    typedef unsigned short  u16;

    typedef int             s32;
    typedef unsigned int    u32;

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

        struct rgba {
            rgba(int r1, int g1, int b1, int a1) :
                r(r1), g(g1), b(b1), a(a1) {}

            rgba() : r(0), g(0), b(0), a(0) {}

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


    // colorize the image, which is w x h, left alpha channel unchanged.
    //
    // it just adds to each pixel in the image
    // aproproriate value.

    void colorizeHSV(const SkBitmap &bitmap,
                     const qreal hue,
                     const qreal saturation,
                     const qreal lightness,
                     const qreal alpha);

    // change brightness of the image
    void brightness(const SkBitmap &bitmap, int bn);
    void anim_brightness(const SkBitmap &bitmap, qreal bn) ;
    // change gamma
    // gamma should be  0.0 <= L <= 6.0
    //
    // it is no problem to set L to 8.0 or 9.0, but the resulting
    // image won't have much difference from 6.0
    void gamma(const SkBitmap &bitmap, qreal L);

    void replaceColor(const SkBitmap &bitmap,
                  const int redR,
                  const int greenR,
                  const int blueR,
                  const int alphaR,
                  const int redT,
                  const int greenT,
                  const int blueT,
                  const int alphaT,
                  const int tolerance,
                  const qreal smooth);

    // change contrast with Photoshop-like method
    // contrast should be  -255 <= contrast <= 255
    void contrast(const SkBitmap &bitmap, int contrast);
    void anim_contrast(const SkBitmap &bitmap, qreal contrast);
    enum swapRGBtype { GBR = 0, BRG = 1 };

    // negative
    void negative(const SkBitmap &bitmap);

    // swap RGB values
    void swapRGB(const SkBitmap &bitmap, int type);

        //
        // All the following filters are ported from KDE's
        // KImageEffect. See kdelibs/kdefx/kimageeffect.cpp
        // for more.
        //

    // blend
        // opacity = [0.0; 1.0]
    void blend(const SkBitmap &bitmap, const rgb &rgb, float opacity);
    void replaceColor(const SkBitmap &bitmap,
                      const int rInt,
                      const int gInt,
                      const int bInt);

        // val = [0.0; 1.0]
        void fade(const SkBitmap &bitmap, const rgb &rgb, float val);

        void gray(const SkBitmap &bitmap);

        // desat = [0.0; 1.0]
        void desaturate(const SkBitmap &bitmap, qreal desat);

        // threshold = [0; 255]
        void threshold(const SkBitmap &bitmap, unsigned int threshold);

        // factor = [0.0; 50.0]
        void solarize(const SkBitmap &bitmap, qreal factor);

        // amount = [1; 10]
        void spread(const SkBitmap &bitmap, unsigned int amount);

        // degrees = [-720.0; 720.0]
        void swirl(const SkBitmap &bitmap, qreal degrees, const rgba &background);

        void noise(const SkBitmap &bitmap, NoiseType noise_type);

        void flatten(const SkBitmap &bitmap, const rgb &ca, const rgb &cb);

        // azimuth = [0.0; 90.0], elevation = [0.0; 90.0]
        void shade(const SkBitmap &bitmap, bool color, qreal azimuth, qreal elevation);

        void equalize(const SkBitmap &bitmap);

        void applyBlur(const SkBitmap &bitmap,
                       const qreal scale,
                       const qreal blurRadius,
                       const bool highQuality,
                       const bool hasKeys);

        void applyBlur(const SkBitmap &bitmapg,
                       const qreal scale,
                       const qreal blurRadius,
                       const bool highQuality,
                       const bool hasKeys,
                       const qreal opacity);

        // radius = [0.01; 90.0], sigma = [0.01; 50.0]
        void anim_fast_blur(const SkBitmap &bitmap, const qreal fRadius);
        void anim_fast_blur(const SkBitmap &bitmap, const qreal fRadius,
                            const qreal opacityT);
        void fast_blur(const SkBitmap &bitmap, int radius);
        void fast_blur(const SkBitmap &bitmap, int radius,
                       const qreal opacityT);

        void anim_fast_shadow(const SkBitmap &bitmap,
                              const qreal fRed,
                              const qreal fGreen,
                              const qreal fBlue,
                              const qreal fDx,
                              const qreal fDy,
                              const qreal fRadius);

        void blur(const SkBitmap &bitmap, qreal radius, qreal sigma);

        void qblurMono(qreal *data,
                       const int w,
                       const int h,
                       const qreal fRadius);
        void qspredMono(qreal *data,
                       const int w,
                       const int h,
                       const qreal fRadius);

        void despeckle(const SkBitmap &bitmap);

        // factor = [0; 100]
        void implode(const SkBitmap &bitmap, qreal factor, const rgba &background);

        // radius = [0.01; 30.0]
        void edge(SkBitmap &bitmap, qreal radius);

        // radius = [0.01; 99.9], sigma = [0.01; 99.9]
        void emboss(SkBitmap &bitmap, qreal radius, qreal sigma);

        // radius = [0.01; 99.9], sigma = [0.01; 30.0]
        void sharpen(SkBitmap &bitmap, qreal radius, qreal sigma);

        // radius = [1.0; 5.0]
        void oil(const SkBitmap &bitmap, qreal radius);

        // red-eye removal.
        // th = [0; 255]
        void redeye(const SkBitmap &bitmap, const int w, const int h,
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
