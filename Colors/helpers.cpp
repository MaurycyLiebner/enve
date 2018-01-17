#include "helpers.h"
#include <math.h>
#include <GL/gl.h>
#include <QString>
#include <sys/sysinfo.h>

void rotate(float rad_t, float *x_t, float *y_t)
{
    float cos_hue = cos(rad_t);
    float sin_hue = sin(rad_t);
    float x_rotated_t = *x_t*cos_hue - *y_t*sin_hue;
    float y_rotated_t = *x_t*sin_hue + *y_t*cos_hue;
    *x_t = x_rotated_t;
    *y_t = y_rotated_t;
}

unsigned char truncateU8(const int &val) {
    if(val > 255) {
        return 255;
    } else if(val < 0) {
        return 0;
    }
    return val;
}

int clampInt(int val, int min, int max) {
    if(val > max) {
        return max;
    } else if(val < min) {
        return min;
    } else {
        return val;
    }
}


//bool isnan(float var)
//{
//    volatile float d = var;
//    return d != d;
//}

void glOrthoAndViewportSet(unsigned int w, unsigned int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, w, h, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
}

float clamp(float val_t, float min_t, float max_t)
{
    if(val_t > max_t)
    {
        return max_t;
    }
    if(val_t < min_t)
    {
        return min_t;
    }
    return val_t;
}

float getAngleF(double x1, double y1, double x2, double y2)
{
    double dot = x1*x2 + y1*y2;
    double det = x1*y2 - y1*x2;
    return atan2(det, dot)*RadToF + 0.5;
}

float getAngleDeg(double x1, double y1, double x2, double y2)
{
    double dot = x1*x2 + y1*y2;
    double det = x1*y2 - y1*x2;
    return atan2(det, dot)*RadToDeg + 180;
}

void normalize(qreal *x_t, qreal *y_t, qreal dest_len) {
    float x_val_t = *x_t;
    float y_val_t = *y_t;
    float curr_len = sqrt(x_val_t*x_val_t + y_val_t*y_val_t);
    *x_t = x_val_t*dest_len/curr_len;
    *y_t = y_val_t*dest_len/curr_len;
}

void rgb_to_hsv_float (float *r_ /*h*/, float *g_ /*s*/, float *b_ /*v*/)
{
  float max, min, delta;
  float h, s, v;
  float r, g, b;

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
void hsv_to_rgb_float (float *h_, float *s_, float *v_)
{
  int    i;
  double f, w, q, t;
  float h, s, v;
  float r, g, b;
  r = g = b = 0.0; // silence gcc warning

  h = *h_;
  s = *s_;
  v = *v_;

  h = h - floor(h);
  s = CLAMP(s, 0.0, 1.0);
  v = CLAMP(v, 0.0, 1.0);

  double hue;

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

      i = (int) hue;
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
void rgb_to_hsl_float (float *r_, float *g_, float *b_)
{
  double max, min, delta;

  float h, s, l;
  float r, g, b;

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

  if (max == min)
    {
      s = 0.0;
      h = 0.0; //GIMP_HSL_UNDEFINED;
    }
  else
    {
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


/**
 * gimp_hsl_to_rgb:
 * @hsl: A color value in the HSL colorspace
 * @rgb: The value converted to a value in the RGB colorspace
 *
 * Convert a HSL color value to an RGB color value.
 **/
void hsl_to_rgb_float (float *h_, float *s_, float *l_)
{
  float h, s, l;
  float r, g, b;

  h = *h_;
  s = *s_;
  l = *l_;

  h = h - floor(h);
  s = CLAMP(s, 0.0, 1.0);
  l = CLAMP(l, 0.0, 1.0);

  if (s == 0)
    {
      /*  achromatic case  */
      r = l;
      g = l;
      b = l;
    }
  else
    {
      double m1, m2;

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

void hsv_to_hsl(float* h, float* s, float *v)
{
    float hh = *h;
    float ss = *s;
    float vv = *v;
    *h = hh;
    *v = (2 - ss) * vv;
    *s = ss * vv;
    *s /= (*v <= 1) ? (*v) : 2 - (*v);
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
    ll *= 2;
    ss *= (ll <= 1) ? ll : 2 - ll;
    *l = (ll + ss) *0.5;
    *s = (2 * ss) / (ll + ss);
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
    float hh = *h;
    float ss = *s;
    float ll = *l;
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

  if (s == 0)
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

      i = (int) hue;
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

  if (max == min)
    {
      s = 0.0;
      h = 0.0; //GIMP_HSL_UNDEFINED;
    }
  else
    {
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

bool isNonZero(const float &val_t) {
    return val_t > 0.0001f || val_t < - 0.0001f;
}

bool isZero(const float val_t) {
    return val_t < 0.0001f && val_t > - 0.0001f;
}


qreal getUNoise(qreal noise_scale)
{
    if( isNonZero(noise_scale) )
    {
        return ( rand() % 101 )*0.01 * noise_scale;
    }
    else
    {
        return 0.f;
    }
}

qreal getNoise(qreal noise_scale)
{
    if( isNonZero(noise_scale) )
    {
        return ( rand() % 201 - 100)*0.01 * noise_scale;
    }
    else
    {
        return 0.f;
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
                  qreal *value_y)
{
    if(isNonZero(noise_t) )
    {
        uchar max_stroke_noise_count = (uchar)(100 - noise_frequency*100);
        if(*noise_count >= max_stroke_noise_count )
        {
            *noise_count = 0;
            *previous_noise_x = *next_noise_x;
            *previous_noise_y = *next_noise_y;
            *next_noise_x = getNoise(noise_t );
            *next_noise_y = getNoise(noise_t );
        }
        else
        {
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
                qreal *value)
{
    if(isNonZero(noise_t) )
    {
        uchar max_stroke_noise_count = (uchar)(100 - noise_frequency*100);
        if(*noise_count >= max_stroke_noise_count )
        {
            *noise_count = 0;
            *previous_noise = *next_noise;
            *next_noise = getNoise(noise_t );
        }
        else
        {
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
                 qreal *value)
{
    if(isNonZero(noise_t) )
    {
        uchar max_stroke_noise_count = (uchar)(10000 - noise_frequency*10000);
        if(*noise_count >= max_stroke_noise_count )
        {
            *noise_count = 0;
            *previous_noise = *next_noise;
            *next_noise = getUNoise(noise_t );
        }
        else
        {
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
#include "Paint/PaintLib/brush.h"
void saveBrushDataAsFile(Brush *brush_t, QString file_path_t)
{
    QFile file(file_path_t);
    file.open(QFile::WriteOnly | QFile::Text);
    file.resize(0);
    QTextStream in(&file);
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        in << brush_t->getSettingAsFileLine(Brush::brush_settings_info[i].setting ) << endl;
    }
    in.flush();
    file.close();
}

void saveBrushDataAsFile(Brush *brush_t, QString collection_name, QString brush_name)
{
    brush_name = brush_name.replace(" ", "_").toLower();
    QFile file("brushes/" + collection_name + "/" + brush_name + ".plb");
    file.open(QFile::WriteOnly | QFile::Text);
    file.resize(0);
    QTextStream in(&file);
    in << "name:" + brush_name << endl;
    for(int i = 0; i < BRUSH_SETTINGS_COUNT; i++)
    {
        in << brush_t->getSettingAsFileLine(Brush::brush_settings_info[i].setting ) << endl;
    }
    in.flush();
    file.close();
}

ushort getFreeRamMB()
{
    struct sysinfo info_t;
    sysinfo(&info_t);
    return info_t.freeram*0.000001;
}
