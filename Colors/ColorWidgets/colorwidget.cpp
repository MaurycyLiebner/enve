#include "colorwidget.h"
#include "Colors/helpers.h"
#include "mainwindow.h"

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

void ColorWidget::setColorHSV_f(GLfloat h, GLfloat s, GLfloat v, bool emit_signal)
{
    hue = h;
    saturation = s;
    value = v;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setHue_f(GLfloat h, bool emit_signal)
{
    hue = h;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setHSVSaturation_f(GLfloat s, bool emit_signal)
{
    saturation = s;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setValue_f(GLfloat v, bool emit_signal)
{
    value = v;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setColorHSV_i(GLushort h, GLushort s, GLushort v, bool emit_signal)
{
    hue = clamp(h/360.f, 0.f, 1.f);
    saturation = clamp(s*0.01f, 0.f, 1.f);
    value = clamp(v*0.01f, 0.f, 1.f);
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setHue_i(GLushort h, bool emit_signal)
{
    hue = clamp(h/360.f, 0.f, 1.f);
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setHSVSaturation_i(GLushort s, bool emit_signal)
{
    saturation = clamp(s*0.01f, 0.f, 1.f);
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setValue_i(GLushort v, bool emit_signal)
{
    value = clamp(v*0.01f, 0.f, 1.f);
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setColorHSL_f(GLfloat h, GLfloat s, GLfloat l, bool emit_signal)
{
    hsl_to_hsv(&h, &s, &l);
    setColorHSV_f(h, s, l, emit_signal);
}

void ColorWidget::setLightness_f(GLfloat l, bool emit_signal)
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
    setColorHSV_f(hue, hsv_s, l, emit_signal);
}

void ColorWidget::setHSLSaturation_f(GLfloat s, bool emit_signal)
{
    GLfloat hsv_h = hue;
    GLfloat hsv_s = saturation;
    GLfloat hsv_v = value;
    hsv_to_hsl(&hsv_h, &hsv_s, &hsv_v);

    hsl_to_hsv(&hsv_h, &s, &hsv_v);

    setColorHSV_f(hue, s, hsv_v, emit_signal);
}

void ColorWidget::setColorHSL_i(GLushort h, GLushort s, GLushort l,
                                bool emit_signal)
{
    Q_UNUSED(emit_signal);
    GLfloat h_f = clamp(h/360.f, 0.f, 1.f);
    GLfloat s_f = clamp(s*0.01f, 0.f, 1.f);
    GLfloat l_f = clamp(l*0.01f, 0.f, 1.f);
    setColorHSL_f(h_f, s_f, l_f);
}

void ColorWidget::setLightness_i(GLushort l, bool emit_signal)
{
    GLfloat l_f = clamp(l*0.01f, 0.f, 1.f);
    setLightness_f(l_f, emit_signal);
}

void ColorWidget::setHSLSaturation_i(GLushort s, bool emit_signal)
{
    GLfloat s_f = clamp(s*0.01f, 0.f, 1.f);
    setHSLSaturation_f(s_f, emit_signal);
}

void ColorWidget::setColorRGB_f(GLfloat r, GLfloat g, GLfloat b, bool emit_signal)
{
    rgb_to_hsv_float(&r, &g, &b);
    hue = r;
    saturation = g;
    value = b;
    emitColorChangedSignal(emit_signal);
}

void ColorWidget::setR_f(GLfloat r, bool emit_signal)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(r, c_g, c_b, emit_signal);
}

void ColorWidget::setG_f(GLfloat g, bool emit_signal)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(c_r, g, c_b, emit_signal);
}

void ColorWidget::setB_f(GLfloat b, bool emit_signal)
{
    GLfloat c_r = hue;
    GLfloat c_g = saturation;
    GLfloat c_b = value;
    hsv_to_rgb_float(&c_r, &c_g, &c_b);
    setColorRGB_f(c_r, c_g, b, emit_signal);
}

void ColorWidget::setR_i(GLushort r, bool emit_signal)
{
    GLfloat r_f = r/255.f;
    setR_f(r_f, emit_signal);
}

void ColorWidget::setG_i(GLushort g, bool emit_signal)
{
    GLfloat g_f = g/255.f;
    setG_f(g_f, emit_signal);
}

void ColorWidget::setB_i(GLushort b, bool emit_signal)
{
    GLfloat b_f = b/255.f;
    setB_f(b_f, emit_signal);
}

void ColorWidget::emitColorChangedSignal(bool external_signal_t)
{
    update();
    if(external_signal_t)
    {
        emit colorChangedHSV(hue, saturation, value);
    }
    else if(internalColorChangeFunc != NULL)
    {
        internalColorChangeFunc(this);
    }
}

void ColorWidget::setInternalColorChangeFunc(void (*internalColorChangeFunc_t)(ColorWidget *))
{
    internalColorChangeFunc = internalColorChangeFunc_t;
}
