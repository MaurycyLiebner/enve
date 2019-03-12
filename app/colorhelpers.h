#ifndef HELPERS_H
#define HELPERS_H
#include "simplemath.h"

enum ColorMode : short {
    RGBMODE,
    HSVMODE,
    HSLMODE
};

enum CVR_TYPE : short {
    CVR_RED,
    CVR_GREEN,
    CVR_BLUE,

    CVR_HUE,
    CVR_HSVSATURATION,
    CVR_VALUE,

    CVR_HSLSATURATION,
    CVR_LIGHTNESS,
    CVR_ALPHA,
    CVR_ALL,
    CVR_NONE
};

extern bool shouldValPointerBeLightHSV(const GLfloat &hue,
                                       const GLfloat &saturation,
                                       const GLfloat &value);
extern bool shouldValPointerBeLightHSL(GLfloat hue,
                                       GLfloat saturation,
                                       GLfloat lightness);
extern bool shouldValPointerBeLightRGB(GLfloat r,
                                       GLfloat g,
                                       GLfloat b);

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


extern qreal getUNoise(qreal noise_scale);
extern qreal getNoise(qreal noise_scale);
extern void applyXYNoise(qreal noise_t,
                         qreal &previous_noise_x,
                         qreal &next_noise_x,
                         qreal &previous_noise_y,
                         qreal &next_noise_y,
                         const qreal &noise_frequency,
                         uchar &noise_count,
                         qreal &value_x,
                         qreal &value_y);
extern void applyUNoise(qreal noise_t,
                        qreal &previous_noise,
                        qreal &next_noise,
                        const qreal &noise_frequency,
                        uchar &noise_count,
                        qreal &value);
extern void applyNoise(qreal noise_t,
                       qreal &previous_noise,
                       qreal &next_noise,
                       const qreal &noise_frequency,
                       uchar &noise_count,
                       qreal &value);


class Brush;

extern void saveBrushDataAsFile(Brush *brush_t,
                                QString file_path_t);

extern void saveBrushDataAsFile(Brush *brush_t,
                                QString collection_name,
                                QString brush_name);

extern unsigned short getFreeRamMB();

#endif // HELPERS_H
