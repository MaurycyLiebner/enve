#ifndef HELPERS_H
#define HELPERS_H

#include <stdint.h>
#include "rng-double.h"

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define ROUND(x) ((int) ((x) + 0.5))
#define SIGN(x) ((x)>0?1:(-1))
#define SQR(x) ((x)*(x))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define MAX3(a, b, c) ((a)>(b)?MAX((a),(c)):MAX((b),(c)))
#define MIN3(a, b, c) ((a)<(b)?MIN((a),(c)):MIN((b),(c)))
#define WGM_EPSILON 0.001

void
hsl_to_rgb_double (double *h_, double *s_, double *l_);
void
rgb_to_hsl_double (double *r_, double *g_, double *b_);

void
hsv_to_rgb_double (double *h_, double *s_, double *v_);

void
rgb_to_hsv_double (double *r_ /*h*/, double *g_ /*s*/, double *b_ /*v*/);

void
hcy_to_rgb_double (double *h_, double *c_, double *y_);

void
rgb_to_hcy_double (double *r_, double *g_, double *b_);

double rand_gauss (RngDouble * rng);

double mod_arith(double a, double N);

double smallest_angular_difference(double angleA, double angleB);

double * mix_colors(double *a, double *b, double fac, double paint_mode);

void
rgb_to_spectral (double r, double g, double b, double *spectral_);

void
spectral_to_rgb (double *spectral, double *rgb_);

#endif // HELPERS_H
