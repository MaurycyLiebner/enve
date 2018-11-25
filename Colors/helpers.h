#ifndef HELPERS_H
#define HELPERS_H
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define ROUND(x) ((int) ((x) + 0.5))
#define SIGN(x) ((x)>0?1:(-1))
#define SQR(x) ((x)*(x))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MAX3(a, b, c) ((a)>(b)?MAX((a),(c)):MAX((b),(c)))
#define MIN3(a, b, c) ((a)<(b)?MIN((a),(c)):MIN((b),(c)))

#define PIf 3.14159265f
#define PI 3.14159265358979323846
#define RadToF 0.5/PI
#define RadToDeg 180/PI
#define sqrt_3 1.73205080757
#define sqrt_2 1.41421356237
#define sqrt_3f 1.73205080757f
#define sqrt_2f 1.41421356237f
#include <QtCore>

extern unsigned char truncateU8(const int &val);
extern int clampInt(int val, int min, int max);
extern void glOrthoAndViewportSet(unsigned int w, unsigned int h);
extern void normalize(qreal *x_t, qreal *y_t, qreal dest_len = 1.);
extern float clamp(float val_t, float min_t, float max_t);
extern double getAngleF(double x1, double y1, double x2, double y2);
extern double getAngleDeg(double x1, double y1, double x2, double y2);

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


extern bool isNonZero(const float &val_t);
extern bool isZero(const float val_t);

extern bool isNonZero(const double &val_t);
extern bool isZero(const double val_t);

extern void rotate(float rad_t, float *x_t, float *y_t);

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
