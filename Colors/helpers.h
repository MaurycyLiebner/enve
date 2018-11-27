#ifndef HELPERS_H
#define HELPERS_H
#include "simplemath.h"

extern void glOrthoAndViewportSet(unsigned int w, unsigned int h);

extern void rgb_to_hsv_float (float *r_ /*h*/, float *g_ /*s*/, float *b_ /*v*/);
extern void hsv_to_rgb_float (float *h_, float *s_, float *v_);
extern void rgb_to_hsl_float (float *r_, float *g_, float *b_);
extern void hsl_to_rgb_float (float *h_, float *s_, float *l_);
extern void hsv_to_hsl(float* h, float* s, float *v);
extern void hsl_to_hsv(float* h, float* s, float *l);

extern void qrgb_to_hsv(double *r_ /*h*/, double *g_ /*s*/, double *b_ /*v*/);
extern void qhsv_to_rgb(double *h_, double *s_, double *v_);
extern void qrgb_to_hsl(double *r_, double *g_, double *b_);
extern void qhsl_to_rgb(double *h_, double *s_, double *l_);
extern void qhsv_to_hsl(double* h, double* s, double *v);
extern void qhsl_to_hsv(double* h, double* s, double *l);


extern qreal getUNoise(qreal noise_scale);
extern qreal getNoise(qreal noise_scale);
extern void applyXYNoise(qreal noise_t,
                         qreal *previous_noise_x,
                         qreal *next_noise_x,
                         qreal *previous_noise_y,
                         qreal *next_noise_y,
                         qreal noise_frequency,
                         uchar *noise_count,
                         qreal *value_x,
                         qreal *value_y);
extern void applyUNoise(qreal noise_t,
                        qreal *previous_noise,
                        qreal *next_noise,
                        qreal noise_frequency,
                        uchar *noise_count,
                        qreal *value);
extern void applyNoise(qreal noise_t,
                       qreal *previous_noise,
                       qreal *next_noise,
                       qreal noise_frequency,
                       uchar *noise_count,
                       qreal *value);


class Brush;

extern void saveBrushDataAsFile(Brush *brush_t,
                                QString file_path_t);

extern void saveBrushDataAsFile(Brush *brush_t,
                                QString collection_name,
                                QString brush_name);

extern unsigned short getFreeRamMB();

#endif // HELPERS_H
