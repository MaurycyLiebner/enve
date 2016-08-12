#include "color.h"
#include "helpers.h"

void Color::updateHSVFromRGB()
{
    gl_h = gl_r;
    gl_s = gl_g;
    gl_v = gl_b;
    rgb_to_hsv_float(&gl_h, &gl_s, &gl_v);
}
void Color::updateQColFromHSV()
{
    qcol.setHsvF(gl_h, gl_s, gl_v, gl_a);
}
void Color::setHSV(GLfloat h, GLfloat s, GLfloat v, GLfloat a)
{
    gl_h = h;
    gl_s = s;
    gl_v = v;
    gl_a = a;
    updateQColFromHSV();
    updateRGBFromHSV();
}

void Color::setRGB(GLfloat r_t, GLfloat g_t, GLfloat b_t)
{
    gl_r = r_t;
    gl_g = g_t;
    gl_b = b_t;
    gl_a = 1.f;
    qcol.setRgbF(r_t, g_t, b_t);
    updateHSVFromRGB();
}
void Color::updateRGBFromHSV()
{
    gl_r = gl_h;
    gl_g = gl_s;
    gl_b = gl_v;
    hsv_to_rgb_float(&gl_r, &gl_g, &gl_b);
}

void Color::setGLColor()
{
    glColor4f(gl_r, gl_g, gl_b, gl_a);
}
void Color::setGLClearColor()
{
    glClearColor(gl_r, gl_g, gl_b, gl_a);
}
void Color::setGLColorR(GLfloat r_t)
{
    gl_r = r_t;
    qcol.setRedF(r_t);
    updateHSVFromRGB();
}
void Color::setGLColorG(GLfloat g_t)
{
    gl_g = g_t;
    qcol.setGreenF(g_t);
    updateHSVFromRGB();
}
void Color::setGLColorB(GLfloat b_t)
{
    gl_b = b_t;
    qcol.setBlueF(b_t);
    updateHSVFromRGB();
}
void Color::setQColor(QColor col_t)
{
    gl_r = col_t.redF();
    gl_g = col_t.greenF();
    gl_b = col_t.blueF();
    qcol = col_t;
    updateHSVFromRGB();
}
void Color::setGLColorA(GLfloat a_t)
{
    gl_a = a_t;
    qcol.setAlpha(a_t*255);
}

Color::Color(GLfloat r_t, GLfloat g_t, GLfloat b_t, GLfloat a_t)
{
    gl_r = r_t;
    gl_g = g_t;
    gl_b = b_t;
    gl_a = a_t;
    qcol.setRgbF(r_t, g_t, b_t, a_t);
    updateHSVFromRGB();
}
Color::Color(int r_t, int g_t, int b_t, int a_t)
{
    qcol.setRgb(r_t, g_t, b_t, a_t);
    gl_r = r_t/255.0f;
    gl_g = g_t/255.0f;
    gl_b = b_t/255.0f;
    gl_a = a_t/255.0f;
    updateHSVFromRGB();
}
Color::Color()
{

}
