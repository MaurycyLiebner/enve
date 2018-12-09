#include "helpers.h"
#include <math.h>
#include <QString>
#include <sys/sysinfo.h>
#include "glhelpers.h"


//bool isnan(float var)
//{
//    volatile float d = var;
//    return d != d;
//}

void glOrthoAndViewportSet(unsigned int w, unsigned int h) {
    glViewport(0, 0, static_cast<GLsizei>(w), static_cast<GLsizei>(h));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., w, h, 0., 0., 1.);
    glMatrixMode(GL_MODELVIEW);
}

void rgb_to_hsv_float (float *r_ /*h*/, float *g_ /*s*/, float *b_ /*v*/) {
  float max, min, delta;
  float h, s, v;
  float r, g, b;

  h = 0.f; // silence gcc warning

  r = *r_;
  g = *g_;
  b = *b_;

  r = CLAMP(r, 0.f, 1.f);
  g = CLAMP(g, 0.f, 1.f);
  b = CLAMP(b, 0.f, 1.f);

  max = MAX3(r, g, b);
  min = MIN3(r, g, b);

  v = max;
  delta = max - min;

  if(delta > 0.0001f) {
      s = delta / max;

      if (r == max)
        {
          h = (g - b) / delta;
          if (h < 0.f)
            h += 6.f;
        }
      else if (g == max)
        {
          h = 2.f + (b - r) / delta;
        }
      else if (b == max)
        {
          h = 4.f + (r - g) / delta;
        }

      h /= 6.f;
    }
  else
    {
      s = 0.f;
      h = 0.f;
    }

  *r_ = h;
  *g_ = s;
  *b_ = v;
}

// (from gimp_hsv_to_rgb)
void hsv_to_rgb_float (float *h_, float *s_, float *v_) {
  int    i;
  float f, w, q, t;
  float h, s, v;
  float r, g, b;
  r = g = b = 0.0; // silence gcc warning

  h = *h_;
  s = *s_;
  v = *v_;

  h = h - floor(h);
  s = CLAMP(s, 0.f, 1.f);
  v = CLAMP(v, 0.f, 1.f);

  float hue;

  if (s == 0.f)
    {
      r = v;
      g = v;
      b = v;
    }
  else
    {
      hue = h;

      if (hue == 1.f)
        hue = 0.f;

      hue *= 6.f;

      i = static_cast<int>(floor(hue));
      f = hue - i;
      w = v * (1.f - s);
      q = v * (1.f - (s * f));
      t = v * (1.f - (s * (1.f - f)));

      switch (i)
        {
        case 0:
          r = v;
          g = t;
          b = w;
          break;
        case 1:
          r = q;
          g = v;
          b = w;
          break;
        case 2:
          r = w;
          g = v;
          b = t;
          break;
        case 3:
          r = w;
          g = q;
          b = v;
          break;
        case 4:
          r = t;
          g = w;
          b = v;
          break;
        case 5:
          r = v;
          g = w;
          b = q;
          break;
        }
    }

  *h_ = r;
  *s_ = g;
  *v_ = b;
}

// (from gimp_rgb_to_hsl)
void rgb_to_hsl_float (float *r_, float *g_, float *b_) {
  float max, min, delta;

  float h, s, l;
  float r, g, b;

  // silence gcc warnings
  h=0.f;

  r = *r_;
  g = *g_;
  b = *b_;

  r = CLAMP(r, 0.f, 1.f);
  g = CLAMP(g, 0.f, 1.f);
  b = CLAMP(b, 0.f, 1.f);

  max = MAX3(r, g, b);
  min = MIN3(r, g, b);

  l = (max + min) / 2.f;

  if (max == min)
    {
      s = 0.f;
      h = 0.f; //GIMP_HSL_UNDEFINED;
    }
  else
    {
      if (l <= 0.5f)
        s = (max - min) / (max + min);
      else
        s = (max - min) / (2.f - max - min);

      delta = max - min;

      if (delta == 0.f)
        delta = 1.f;

      if (r == max)
        {
          h = (g - b) / delta;
        }
      else if (g == max)
        {
          h = 2.f + (b - r) / delta;
        }
      else if (b == max)
        {
          h = 4.f + (r - g) / delta;
        }

      h /= 6.f;

      if (h < 0.f)
        h += 1.f;
    }

  *r_ = h;
  *g_ = s;
  *b_ = l;
}

static double hsl_value (double n1,
           double n2,
           double hue)
{
  double val;

  if (hue > 6.0)
    hue -= 6.0;
  else if (hue < 0.0)
    hue += 6.0;

  if (hue < 1.0)
    val = n1 + (n2 - n1) * hue;
  else if (hue < 3.0)
    val = n2;
  else if (hue < 4.0)
    val = n1 + (n2 - n1) * (4.0 - hue);
  else
    val = n1;

  return val;
}


static float hsl_value (float n1,
           float n2,
           float hue) {
  float val;

  if (hue > 6.f)
    hue -= 6.f;
  else if (hue < 0.f)
    hue += 6.f;

  if (hue < 1.f)
    val = n1 + (n2 - n1) * hue;
  else if (hue < 3.f)
    val = n2;
  else if (hue < 4.f)
    val = n1 + (n2 - n1) * (4.f - hue);
  else
    val = n1;

  return val;
}

/**
 * gimp_hsl_to_rgb:
 * @hsl: A color value in the HSL colorspace
 * @rgb: The value converted to a value in the RGB colorspace
 *
 * Convert a HSL color value to an RGB color value.
 **/
void hsl_to_rgb_float (float *h_, float *s_, float *l_) {
  float h, s, l;
  float r, g, b;

  h = *h_;
  s = *s_;
  l = *l_;

  h = h - floor(h);
  s = CLAMP(s, 0.f, 1.f);
  l = CLAMP(l, 0.f, 1.f);

  if (s == 0.f)
    {
      /*  achromatic case  */
      r = l;
      g = l;
      b = l;
    }
  else
    {
      float m1, m2;

      if (l <= 0.5f)
        m2 = l * (1.f + s);
      else
        m2 = l + s - l * s;

      m1 = 2.f * l - m2;

      r = hsl_value (m1, m2, h * 6.f + 2.f);
      g = hsl_value (m1, m2, h * 6.f);
      b = hsl_value (m1, m2, h * 6.f - 2.f);
    }

  *h_ = r;
  *s_ = g;
  *l_ = b;
}

void hsv_to_hsl(float* h, float* s, float *v)
{
    float hh = *h;
    float ss = *s;
    float vv = *v;
    *h = hh;
    *v = (2.f - ss) * vv;
    *s = ss * vv;
    *s /= (*v <= 1.f) ? (*v) : 2.f - (*v);
    *v *= 0.5f;
    if(isnan(*s) )
    {
        *s = 0.f;
    }
}

void hsl_to_hsv(float* h, float *s, float *l)
{
    float hh = *h;
    float ss = *s;
    float ll = *l;
    *h = hh;
    ll *= 2.f;
    ss *= (ll <= 1.f) ? ll : 2.f - ll;
    *l = (ll + ss) *0.5f;
    *s = (2.f * ss) / (ll + ss);
    if(isnan(*s) )
    {
        *s = 0.f;
    }
}

void qhsv_to_hsl(qreal* h, qreal* s, qreal *v)
{
    qreal hh = *h;
    qreal ss = *s;
    qreal vv = *v;
    *h = hh;
    *v = (2 - ss) * vv;
    *s = ss * vv;
    *s /= (*v <= 1) ? (*v) : 2 - (*v);
    *v *= 0.5;
    if(isnan(*s) )
    {
        *s = 0.;
    }
}

void qhsl_to_hsv(qreal* h, qreal *s, qreal *l)
{
    qreal hh = *h;
    qreal ss = *s;
    qreal ll = *l;
    *h = hh;
    ll *= 2;
    ss *= (ll <= 1) ? ll : 2 - ll;
    *l = (ll + ss) *0.5;
    *s = (2 * ss) / (ll + ss);
    if(isnan(*s) )
    {
        *s = 0.;
    }
}

void qhsl_to_rgb(qreal *h_, qreal *s_, qreal *l_)
{
  qreal h, s, l;
  qreal r, g, b;

  h = *h_;
  s = *s_;
  l = *l_;

  h = h - floor(h);
  s = CLAMP(s, 0.0, 1.0);
  l = CLAMP(l, 0.0, 1.0);

  if (s == 0.)
    {
      /*  achromatic case  */
      r = l;
      g = l;
      b = l;
    }
  else
    {
      qreal m1, m2;

      if (l <= 0.5)
        m2 = l * (1.0 + s);
      else
        m2 = l + s - l * s;

      m1 = 2.0 * l - m2;

      r = hsl_value (m1, m2, h * 6.0 + 2.0);
      g = hsl_value (m1, m2, h * 6.0);
      b = hsl_value (m1, m2, h * 6.0 - 2.0);
    }

  *h_ = r;
  *s_ = g;
  *l_ = b;
}

void qrgb_to_hsv(qreal *r_ /*h*/, qreal *g_ /*s*/, qreal *b_ /*v*/)
{
  qreal max, min, delta;
  qreal h, s, v;
  qreal r, g, b;

  h = 0.0; // silence gcc warning

  r = *r_;
  g = *g_;
  b = *b_;

  r = CLAMP(r, 0.0, 1.0);
  g = CLAMP(g, 0.0, 1.0);
  b = CLAMP(b, 0.0, 1.0);

  max = MAX3(r, g, b);
  min = MIN3(r, g, b);

  v = max;
  delta = max - min;

  if (delta > 0.0001)
    {
      s = delta / max;

      if (r == max)
        {
          h = (g - b) / delta;
          if (h < 0.0)
            h += 6.0;
        }
      else if (g == max)
        {
          h = 2.0 + (b - r) / delta;
        }
      else if (b == max)
        {
          h = 4.0 + (r - g) / delta;
        }

      h /= 6.0;
    }
  else
    {
      s = 0.0;
      h = 0.0;
    }

  *r_ = h;
  *g_ = s;
  *b_ = v;
}

// (from gimp_hsv_to_rgb)
void qhsv_to_rgb(qreal *h_, qreal *s_, qreal *v_)
{
  int    i;
  qreal f, w, q, t;
  qreal h, s, v;
  qreal r, g, b;
  r = g = b = 0.0; // silence gcc warning

  h = *h_;
  s = *s_;
  v = *v_;

  h = h - floor(h);
  s = CLAMP(s, 0.0, 1.0);
  v = CLAMP(v, 0.0, 1.0);

  qreal hue;

  if (s == 0.0)
    {
      r = v;
      g = v;
      b = v;
    }
  else
    {
      hue = h;

      if (hue == 1.0)
        hue = 0.0;

      hue *= 6.0;

      i = static_cast<int>(floor(hue));
      f = hue - i;
      w = v * (1.0 - s);
      q = v * (1.0 - (s * f));
      t = v * (1.0 - (s * (1.0 - f)));

      switch (i)
        {
        case 0:
          r = v;
          g = t;
          b = w;
          break;
        case 1:
          r = q;
          g = v;
          b = w;
          break;
        case 2:
          r = w;
          g = v;
          b = t;
          break;
        case 3:
          r = w;
          g = q;
          b = v;
          break;
        case 4:
          r = t;
          g = w;
          b = v;
          break;
        case 5:
          r = v;
          g = w;
          b = q;
          break;
        }
    }

  *h_ = r;
  *s_ = g;
  *v_ = b;
}

// (from gimp_rgb_to_hsl)
void qrgb_to_hsl(qreal *r_, qreal *g_, qreal *b_)
{
  qreal max, min, delta;

  qreal h, s, l;
  qreal r, g, b;

  // silence gcc warnings
  h=0;

  r = *r_;
  g = *g_;
  b = *b_;

  r = CLAMP(r, 0.0, 1.0);
  g = CLAMP(g, 0.0, 1.0);
  b = CLAMP(b, 0.0, 1.0);

  max = MAX3(r, g, b);
  min = MIN3(r, g, b);

  l = (max + min) / 2.0;

  if(max == min) {
      s = 0.0;
      h = 0.0; //GIMP_HSL_UNDEFINED;
    } else {
      if (l <= 0.5)
        s = (max - min) / (max + min);
      else
        s = (max - min) / (2.0 - max - min);

      delta = max - min;

      if (delta == 0.0)
        delta = 1.0;

      if (r == max)
        {
          h = (g - b) / delta;
        }
      else if (g == max)
        {
          h = 2.0 + (b - r) / delta;
        }
      else if (b == max)
        {
          h = 4.0 + (r - g) / delta;
        }

      h /= 6.0;

      if (h < 0.0)
        h += 1.0;
    }

  *r_ = h;
  *g_ = s;
  *b_ = l;
}


qreal getUNoise(qreal noise_scale) {
    if(isNonZero(noise_scale)) {
        return ( rand() % 101 )*0.01 * noise_scale;
    } else {
        return 0.;
    }
}

qreal getNoise(qreal noise_scale) {
    if(isNonZero(noise_scale)) {
        return ( rand() % 201 - 100)*0.01 * noise_scale;
    } else {
        return 0.;
    }
}

void applyXYNoise(qreal noise_t,
                  qreal *previous_noise_x,
                  qreal *next_noise_x,
                  qreal *previous_noise_y,
                  qreal *next_noise_y,
                  qreal noise_frequency,
                  uchar *noise_count,
                  qreal *value_x,
                  qreal *value_y) {
    if(isNonZero(noise_t)) {
        uchar max_stroke_noise_count = static_cast<uchar>(100 - noise_frequency*100);
        if(*noise_count >= max_stroke_noise_count) {
            *noise_count = 0;
            *previous_noise_x = *next_noise_x;
            *previous_noise_y = *next_noise_y;
            *next_noise_x = getNoise(noise_t );
            *next_noise_y = getNoise(noise_t );
        } else {
            *noise_count = *noise_count + 1;
        }
        qreal current_noise_x = ( (*next_noise_x)*(*noise_count) +
                                  (*previous_noise_x)*
                                  (max_stroke_noise_count - (*noise_count) )
                                  )/max_stroke_noise_count;
        *value_x += current_noise_x;
        qreal current_noise_y = ( (*next_noise_y)*(*noise_count) +
                                  (*previous_noise_y)*
                                  (max_stroke_noise_count - (*noise_count) )
                                  )/max_stroke_noise_count;
        *value_y += current_noise_y;
    }
}

void applyNoise(qreal noise_t,
                qreal *previous_noise,
                qreal *next_noise,
                qreal noise_frequency,
                uchar *noise_count,
                qreal *value) {
    if(isNonZero(noise_t) ) {
        uchar max_stroke_noise_count = static_cast<uchar>(100 - noise_frequency*100);
        if(*noise_count >= max_stroke_noise_count) {
            *noise_count = 0;
            *previous_noise = *next_noise;
            *next_noise = getNoise(noise_t );
        } else {
            *noise_count = *noise_count + 1;
        }
        qreal current_noise = ( (*next_noise)*(*noise_count) +
                                  (*previous_noise)*
                                  (max_stroke_noise_count - (*noise_count) )
                                  )/max_stroke_noise_count;
        *value += current_noise;
    }
}

void applyUNoise(qreal noise_t,
                 qreal *previous_noise,
                 qreal *next_noise,
                 qreal noise_frequency,
                 uchar *noise_count,
                 qreal *value) {
    if(isNonZero(noise_t)) {
        uchar max_stroke_noise_count = static_cast<uchar>(10000 - noise_frequency*10000);
        if(*noise_count >= max_stroke_noise_count) {
            *noise_count = 0;
            *previous_noise = *next_noise;
            *next_noise = getUNoise(noise_t );
        } else {
            *noise_count = *noise_count + 1;
        }
        qreal current_noise = ( (*next_noise)*(*noise_count) +
                                  (*previous_noise)*
                                  (max_stroke_noise_count - (*noise_count) )
                                  )/max_stroke_noise_count;
        *value += current_noise;
    }
}

#include <QInputDialog>
#include <QString>
#include <QFile>
#include <QTextStream>
#include "Paint/brush.h"
void saveBrushDataAsFile(Brush *brush_t, QString file_path_t) {
    QFile file(file_path_t);
    file.open(QFile::WriteOnly);
    file.resize(0);
    brush_t->writeBrush(&file);
    file.close();
}

void saveBrushDataAsFile(Brush *brush_t,
                         QString collection_name,
                         QString brush_name) {
    brush_name = brush_name.replace(" ", "_").toLower();
    QFile file("brushes/" + collection_name + "/" + brush_name + ".avb");
    file.open(QFile::WriteOnly);
    file.resize(0);
    brush_t->writeBrush(&file);
    file.close();
}

ushort getFreeRamMB() {
    struct sysinfo info_t;
    sysinfo(&info_t);
    return static_cast<ushort>(info_t.freeram/1000000);
}
