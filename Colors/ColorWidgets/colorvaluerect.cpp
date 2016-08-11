#include "colorvaluerect.h"
#include "Colors/helpers.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>

void updateValExt(ColorWidget *color_widget_t)
{
    ( (ColorValueRect*) color_widget_t)->updateVal();
}

ColorValueRect::ColorValueRect(CVR_TYPE type_t, QWidget *parent) : ColorWidget(parent)
{
    type = type_t;
    setInternalColorChangeFunc(&updateValExt);
}

void ColorValueRect::paintGL()
{
    GLfloat r = hue;
    GLfloat g = saturation;
    GLfloat b = value;
    hsv_to_rgb_float(&r, &g, &b);
    bool text_light = false;
    if(type == CVR_RED)
    {
        text_light = shouldValPointerBeLightRGB(0.f, g, b);
        drawRect(0.f, 0.f, width(), height(),
                 0.f, g, b,
                 1.f, g, b,
                 1.f, g, b,
                 0.f, g, b);
    }
    else if(type == CVR_GREEN)
    {
        text_light = shouldValPointerBeLightRGB(r, 0.f, b);
        drawRect(0.f, 0.f, width(), height(),
                 r, 0.f, b,
                 r, 1.f, b,
                 r, 1.f, b,
                 r, 0.f, b);
    }
    else if(type == CVR_BLUE)
    {
        text_light = shouldValPointerBeLightRGB(r, g, 0.f);
        drawRect(0.f, 0.f, width(), height(),
                 r, g, 0.f,
                 r, g, 1.f,
                 r, g, 1.f,
                 r, g, 0.f);
    }
    else if(type == CVR_HUE)
    {
        text_light = shouldValPointerBeLightHSV(0.f, saturation, value);
        GLfloat seg_width = (GLfloat)width()/number_segments;
        GLfloat last_r;
        GLfloat last_g;
        GLfloat last_b;
        GLfloat hue_per_i = 1.f/number_segments;
        for(int i = 0; i <= number_segments; i++)
        {
            GLfloat c_r = i*hue_per_i;
            GLfloat c_g = saturation;
            GLfloat c_b = value;
            hsv_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0)
            {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == number_segments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    }
    else if(type == CVR_HSVSATURATION)
    {
        text_light = shouldValPointerBeLightHSV(hue, 0.f, value);
        GLfloat seg_width = (GLfloat)width()/number_segments;
        GLfloat last_r;
        GLfloat last_g;
        GLfloat last_b;
        GLfloat saturation_per_i = 1.f/number_segments;
        for(int i = 0; i <= number_segments; i++)
        {
            GLfloat c_r = hue;
            GLfloat c_g = i*saturation_per_i;
            GLfloat c_b = value;
            hsv_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0)
            {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == number_segments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    }
    else if(type == CVR_VALUE)
    {
        text_light = shouldValPointerBeLightHSV(hue, saturation, 0.f);
        GLfloat seg_width = (GLfloat)width()/number_segments;
        GLfloat last_r;
        GLfloat last_g;
        GLfloat last_b;
        GLfloat value_per_i = 1.f/number_segments;
        for(int i = 0; i <= number_segments; i++)
        {
            GLfloat c_r = hue;
            GLfloat c_g = saturation;
            GLfloat c_b = i*value_per_i;
            hsv_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0)
            {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == number_segments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    }
    else if(type == CVR_HSLSATURATION)
    {
        GLfloat seg_width = (GLfloat)width()/number_segments;
        GLfloat last_r;
        GLfloat last_g;
        GLfloat last_b;
        GLfloat saturation_per_i = 1.f/number_segments;

        GLfloat h_t = hue;
        GLfloat s_t = saturation;
        GLfloat l_t = value;
        hsv_to_hsl(&h_t, &s_t, &l_t);
        text_light = shouldValPointerBeLightHSL(h_t, 0.f, l_t);
        for(int i = 0; i <= number_segments; i++)
        {
            GLfloat c_r = hue;
            GLfloat c_g = i*saturation_per_i;
            GLfloat c_b = l_t;
            hsl_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0)
            {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == number_segments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    }
    else if(type == CVR_LIGHTNESS)
    {
        GLfloat seg_width = (GLfloat)width()/number_segments;
        GLfloat last_r;
        GLfloat last_g;
        GLfloat last_b;
        GLfloat lightness_per_i = 1.f/number_segments;

        GLfloat h_t = hue;
        GLfloat s_t = saturation;
        GLfloat l_t = value;
        hsv_to_hsl(&h_t, &s_t, &l_t);
        text_light = shouldValPointerBeLightHSL(h_t, s_t, 0.f);
        for(int i = 0; i <= number_segments; i++)
        {
            GLfloat c_r = hue;
            GLfloat c_g = s_t;
            GLfloat c_b = i*lightness_per_i;
            hsl_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0)
            {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == number_segments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    }

    if(shouldValPointerBeLightHSV(hue, saturation, value) )
    {
        drawSolidRectCenter(val*width(), height()*0.5, 4.f, height(),
                      1.f, 1.f, 1.f, false, false, false, false);

    }
    else
    {
        drawSolidRectCenter(val*width(), height()*0.5, 4.f, height(),
                      0.f, 0.f, 0.f, false, false, false, false);

    }
}

void ColorValueRect::mouseMoveEvent(QMouseEvent *e)
{
    mouseInteraction(e->x());
}

void ColorValueRect::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton)
    {
        return;
    }
    mouseInteraction(e->x());
}

void ColorValueRect::mouseReleaseEvent(QMouseEvent *)
{
    hsl_saturaton_tmp = -1.f;
}

void ColorValueRect::wheelEvent(QWheelEvent *e)
{
    if(e->delta() > 0)
    {
        setVal(clamp(val + 0.01f, 0.f, 1.f), true);
    }
    else
    {
        setVal(clamp(val - 0.01f, 0.f, 1.f), true);
    }
    setColorParameterFromVal();
}

void ColorValueRect::mouseInteraction(int x_t)
{
    setVal(clamp( ((GLfloat)x_t)/width(), 0.f, 1.f), true);
    setColorParameterFromVal();
}

void ColorValueRect::setColorParameterFromVal()
{
    if(type == CVR_RED)
    {
        setR_f(val, true);
    }
    else if(type == CVR_GREEN)
    {
        setG_f(val, true);
    }
    else if(type == CVR_BLUE)
    {
        setB_f(val, true);
    }
    else if(type == CVR_HUE)
    {
        setHue_f(val, true);
    }
    else if(type == CVR_HSVSATURATION)
    {
        setHSVSaturation_f(val, true);
    }
    else if(type == CVR_VALUE)
    {
        setValue_f(val, true);
    }
    else if(type == CVR_HSLSATURATION)
    {
        setHSLSaturation_f(val, true);
    }
    else if(type == CVR_LIGHTNESS)
    {
        setLightness_f(val, true);
    }
}

void ColorValueRect::updateVal()
{
    if(type == CVR_RED)
    {
        GLfloat r_t = hue;
        GLfloat g_t = saturation;
        GLfloat b_t = value;
        hsv_to_rgb_float(&r_t, &g_t, &b_t);
        setVal(r_t);
    }
    else if(type == CVR_GREEN)
    {
        GLfloat r_t = hue;
        GLfloat g_t = saturation;
        GLfloat b_t = value;
        hsv_to_rgb_float(&r_t, &g_t, &b_t);
        setVal(g_t);
    }
    else if(type == CVR_BLUE)
    {
        GLfloat r_t = hue;
        GLfloat g_t = saturation;
        GLfloat b_t = value;
        hsv_to_rgb_float(&r_t, &g_t, &b_t);
        setVal(b_t);
    }
    else if(type == CVR_HUE)
    {
        setVal(hue);
    }
    else if(type == CVR_HSVSATURATION)
    {
        setVal(saturation);
    }
    else if(type == CVR_VALUE)
    {
        setVal(value);
    }
    else if(type == CVR_HSLSATURATION)
    {
        GLfloat hue_t = hue;
        GLfloat saturation_t = saturation;
        GLfloat lightness_t = value;
        hsv_to_hsl(&hue_t, &saturation_t, &lightness_t);
        setVal(saturation_t);
    }
    else if(type == CVR_LIGHTNESS)
    {
        GLfloat hue_t = hue;
        GLfloat saturation_t = saturation;
        GLfloat lightness_t = value;
        hsv_to_hsl(&hue_t, &saturation_t, &lightness_t);
        setVal(lightness_t);
    }
    emit valUpdated(val);
}

void ColorValueRect::setVal(GLfloat val_t, bool emit_signal)
{
    val = val_t;
    update();
    if(emit_signal)
    {
        emit valChanged(val);
    }
}

void ColorValueRect::setValAndEmitSignal(GLfloat val_t)
{
    setVal(val_t);
    setColorParameterFromVal();
    emit colorChangedHSV(hue, saturation, value);
}
