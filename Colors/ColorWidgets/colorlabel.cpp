#include "colorlabel.h"
#include "Colors/helpers.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWidget>

ColorLabel::ColorLabel(QWidget *parent) : ColorWidget(parent)
{
    //setMinimumSize(80, 20);
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void ColorLabel::mousePressEvent(QMouseEvent *e) {
    Q_UNUSED(e);
}

void ColorLabel::setAlpha(GLfloat alpha_t) {
    alpha = alpha_t;
    update();
}

void ColorLabel::paintGL()
{
    GLfloat half_width = width()*0.5f;
    GLfloat height_t = height();
    GLfloat r = hue;
    GLfloat g = saturation;
    GLfloat b = value;
    hsv_to_rgb_float(&r, &g, &b);

    GLfloat val1 = 0.5f;
    GLfloat val2 = 0.25f;
    for(int i = 0; i < half_width; i += 7) {
        for(int j = 0; j < height_t; j += 7) {
            GLfloat val = ((i + j) % 2 == 0) ? val1 : val2;
            drawSolidRect(i, j, 7, 7, val, val, val,
                          false, false, false, false);
        }
    }
    drawSolidRect(0.f, 0.f, half_width, height_t,
                  r, g, b, alpha,
                  false, false, false, false);
    drawSolidRect(half_width, 0.f, half_width, height_t,
                  r, g, b,
                  false, false, false, false);
}
