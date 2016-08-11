#include "colorlabel.h"
#include "Colors/helpers.h"
#include <QPainter>
#include <QMouseEvent>

ColorLabel::ColorLabel(GLfloat h_t, GLfloat s_t, GLfloat v_t, QWidget *parent) : ColorWidget(parent)
{
    setLastColorHSV(h_t, s_t, v_t);
    //setMinimumSize(80, 20);
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void ColorLabel::saveCurrentColorAsLast()
{
    last_color_h = hue;
    last_color_s = saturation;
    last_color_v = value;
    GLfloat r = hue;
    GLfloat g = saturation;
    GLfloat b = value;
    hsv_to_rgb_float(&r, &g, &b);
    last_color_r = r;
    last_color_g = g;
    last_color_b = b;
    update();
}

void ColorLabel::setLastColorHSV(GLfloat h, GLfloat s, GLfloat v)
{
    last_color_h = h;
    last_color_s = s;
    last_color_v = v;
    hsv_to_rgb_float(&h, &s, &v);
    last_color_r = h;
    last_color_g = s;
    last_color_b = v;
}

void ColorLabel::mousePressEvent(QMouseEvent *e)
{
    if(e->x() > width()/2 )
    {
        setColorHSV_f(last_color_h, last_color_s, last_color_v, true);
    }
}

void ColorLabel::paintGL()
{
    GLfloat half_width = width()*0.5;
    GLfloat height_t = height();
    GLfloat r = hue;
    GLfloat g = saturation;
    GLfloat b = value;
    hsv_to_rgb_float(&r, &g, &b);
    drawSolidRect(0.f, 0, half_width, height_t,
                  r, g, b,
                  false, false, false, false);
    drawSolidRect(half_width, 0, half_width, height_t,
                  last_color_r, last_color_g, last_color_b,
                  false, false, false, false);
}
