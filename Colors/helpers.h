#ifndef HELPERS_H
#define HELPERS_H
#include <GL/gl.h>
#include <QString>
#include <sys/sysinfo.h>
#include <QPointF>

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define ROUND(x) ((int) ((x) + 0.5))
#define SIGN(x) ((x)>0?1:(-1))
#define SQR(x) ((x)*(x))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MAX3(a, b, c) ((a)>(b)?MAX((a),(c)):MAX((b),(c)))
#define MIN3(a, b, c) ((a)<(b)?MIN((a),(c)):MIN((b),(c)))

#define PI 3.14159265
#define RadToF 0.5/PI
#define RadToDeg 180/PI
#define sqrt_3 sqrt(3)
#define sqrt_2 sqrt(2);

extern void glOrthoAndViewportSet(GLuint w, GLuint h);
extern void normalize(GLfloat *x_t, GLfloat *y_t, GLfloat dest_len = 1.f);
extern float clamp(float val_t, float min_t, float max_t);
extern float getAngleF(double x1, double y1, double x2, double y2);
extern float getAngleDeg(double x1, double y1, double x2, double y2);

extern void rgb_to_hsv_float (float *r_ /*h*/, float *g_ /*s*/, float *b_ /*v*/);
extern void hsv_to_rgb_float (float *h_, float *s_, float *v_);
extern void rgb_to_hsl_float (float *r_, float *g_, float *b_);
extern void hsl_to_rgb_float (float *h_, float *s_, float *l_);
extern void hsv_to_hsl(float* h, float* s, float *v);
extern void hsl_to_hsv(float* h, float* s, float *l);


extern bool isNonZero(GLfloat val_t);
extern bool isZero(GLfloat val_t);
extern GLfloat getUNoise(GLfloat noise_scale);
extern GLfloat getNoise(GLfloat noise_scale);
extern void applyXYNoise(GLfloat noise_t,
                  GLfloat *previous_noise_x, GLfloat *next_noise_x,
                  GLfloat *previous_noise_y, GLfloat *next_noise_y,
                  GLfloat noise_frequency, uchar *noise_count,
                  GLfloat *value_x, GLfloat *value_y);
extern void applyUNoise(GLfloat noise_t, GLfloat *previous_noise, GLfloat *next_noise,
                GLfloat noise_frequency, uchar *noise_count, GLfloat *value);
extern void applyNoise(GLfloat noise_t, GLfloat *previous_noise, GLfloat *next_noise,
                GLfloat noise_frequency, uchar *noise_count, GLfloat *value);


class Brush;

extern ushort getFreeRamMB();

extern qreal sign(qreal x, qreal y, QPointF p2, QPointF p3);


extern bool pointInTriangle(qreal x, qreal y, QPointF v1, QPointF v2, QPointF v3);

extern bool insideCircle(int r, int x_t, int y_t);

extern bool outsideCircle(int r, int x_t, int y_t);

extern void rotate(float rad_t, float *x_t, float *y_t);

#endif // HELPERS_H
