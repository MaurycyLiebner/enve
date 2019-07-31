#ifndef HELPERS_H
#define HELPERS_H
#include "simplemath.h"

enum class ColorMode : short {
    rgb, hsv, hsl
};

enum class ColorParameter : short {
    none,
    red, green, blue,
    hue, hsvSaturaton, value,
    hslSaturation, lightness,
    alpha,
    colorMode,
    all
};

extern bool shouldValPointerBeLightHSV(const float h,
                                       const float s,
                                       const float v);
extern bool shouldValPointerBeLightHSL(float h, float s, float l);
extern bool shouldValPointerBeLightRGB(float r, float g, float b);

extern void rgb_to_hsv_float (float &r_ /*h*/, float &g_ /*s*/, float &b_ /*v*/);
extern void hsv_to_rgb_float (float &h_, float &s_, float &v_);
extern void rgb_to_hsl_float (float &r_, float &g_, float &b_);
extern void hsl_to_rgb_float (float &h_, float &s_, float &l_);
extern void hsv_to_hsl(float &h, float &s, float &v);
extern void hsl_to_hsv(float &h, float &s, float &l);

extern void qrgb_to_hsv(qreal &r_ /*h*/, qreal &g_ /*s*/, qreal &b_ /*v*/);
extern void qhsv_to_rgb(qreal &h_, qreal &s_, qreal &v_);
extern void qrgb_to_hsl(qreal &r_, qreal &g_, qreal &b_);
extern void qhsl_to_rgb(qreal &h_, qreal &s_, qreal &l_);
extern void qhsv_to_hsl(qreal &h, qreal &s, qreal &v);
extern void qhsl_to_hsv(qreal &h, qreal &s, qreal &l);

#endif // HELPERS_H
