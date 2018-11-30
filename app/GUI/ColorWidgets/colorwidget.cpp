#include "colorwidget.h"
#include "GUI/ColorWidgets/helpers.h"
#include "global.h"

bool shouldValPointerBeLightHSV(GLfloat hue, GLfloat saturation, GLfloat value)
{
    return value < 0.6f || (hue > 0.6f && (saturation > 0.5f || value < 0.7f) );
}

bool shouldValPointerBeLightHSL(GLfloat hue, GLfloat saturation, GLfloat lightness)
{
    hsl_to_hsv(&hue, &saturation, &lightness);
    return shouldValPointerBeLightHSV(hue, saturation, lightness);
}

bool shouldValPointerBeLightRGB(GLfloat r, GLfloat g, GLfloat b)
{
    rgb_to_hsv_float(&r, &g, &b);
    return shouldValPointerBeLightHSV(r, g, b);
}

ColorWidget::ColorWidget(QWidget *parent) : GLWidget(parent)
{

}

void ColorWidget::setColorHSV_f(GLfloat h, GLfloat s, GLfloat v) {
    //if(s > 0.0001) {
    hue = h;
    //}
    saturation = s;
    value = v;
    update();
}

void ColorWidget::setHue_f(GLfloat h)
{
    hue = h;
    update();
}

void ColorWidget::setHSVSaturation_f(GLfloat s)
{
    saturation = s;
    update();
}

void ColorWidget::setValue_f(GLfloat v)
{
    value = v;
    update();
}

void ColorWidget::setColorHSV_i(GLushort h, GLushort s, GLushort v)
{
    hue = clamp(h/360.f, 0.f, 1.f);
    saturation = clamp(s*0.01f, 0.f, 1.f);
    value = clamp(v*0.01f, 0.f, 1.f);
    update();
}

void ColorWidget::setHue_i(GLushort h)
{
    hue = clamp(h/360.f, 0.f, 1.f);
    update();
}

void ColorWidget::setHSVSaturation_i(GLushort s)
{
    saturation = clamp(s*0.01f, 0.f, 1.f);
    update();
}

void ColorWidget::setValue_i(GLushort v)
{
    value = clamp(v*0.01f, 0.f, 1.f);
    update();
}

void ColorWidget::setColorHSL_f(GLfloat h, GLfloat s, GLfloat l)
{
    hsl_to_hsv(&h, &s, &l);
    setColorHSV_f(h, s, l);
}

void ColorWidget::setLightness_f(GLfloat l)
{
    GLfloat hsv_h = hue;
    GLfloat hsv_s = saturation;
    GLfloat hsv_v = value;
    hsv_to_hsl(&hsv_h, &hsv_s, &hsv_v);
    if(hsl_saturaton_tmp > -0.5f)
    {
        hsv_s = hsl_saturaton_tmp;
    }
    else
    {
        hsl_saturaton_tmp = hsv_s;
    }
    hsl_to_hsv(&hsv_h, &hsv_s, &l);
    setColorHSV_f(hue, hsv_s, l);
}

void ColorWidget::setHSLSaturation_f(GLfloat s)
{
    GLfloat hsv_h = hue;
    GLfloat hsv_s = saturation;
    GLfloat hsv_v = value;
    hsv_to_hsl(&hsv_h, &hsv_s, &hsv_v);

    hsl_to_hsv(&hsv_h, &s, &hsv_v);

    setColorHSV_f(hue, s, hsv_v);
}

void ColorWidget::setColorHSL_i(GLushort h, GLushort s, GLushort l)
{
    GLfloat h_f = clamp(h/360.f, 0.f, 1.f);
    GLfloat s_f = clamp(s*0.01f, 0.f, 1.f);
    GLfloat l_f = clamp(l*0.01f, 0.f, 1.f);
    setColorHSL_f(h_f, s_f, l_f);
}

void ColorWidget::setLightness_i(GLushort l)
{
    GLfloat l_f = clamp(l*0.01f, 0.f, 1.f);
    setLightness_f(l_f);
}

void ColorWidget::setHSLSaturation_i(GLushort s)
{
    GLfloat s_f = clamp(s*0.01f, 0.f, 1.f);
    setHSLSaturation_f(s_f);
}

void ColorWidget::setColorRGB_f(GLfloat r, GLfloat g, GLfloat b)
{
    rgb_to_hsv_float(&r, &g, &b);
    hue = r;
    saturation = g;
    value = b;
    update();
}

void ColorWidget::setR_f(GLfloat r)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(r, c_g, c_b);
}

void ColorWidget::setG_f(GLfloat g)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(c_r, g, c_b);
}

void ColorWidget::setB_f(GLfloat b)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(c_r, c_g, b);
}

void ColorWidget::setR_i(GLushort r)
{
    GLfloat r_f = r/255.f;
    setR_f(r_f);
}

void ColorWidget::setG_i(GLushort g)
{
    GLfloat g_f = g/255.f;
    setG_f(g_f);
}

void ColorWidget::setB_i(GLushort b)
{
    GLfloat b_f = b/255.f;
    setB_f(b_f);
}
