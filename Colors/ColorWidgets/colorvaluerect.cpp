#include "colorvaluerect.h"
#include "Colors/helpers.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include "mainwindow.h"

ColorValueRect::ColorValueRect(CVR_TYPE type_t, QWidget *parent) : ColorWidget(parent)
{
    type = type_t;
}

void ColorValueRect::paintGL()
{
    GLfloat r = hue;
    GLfloat g = saturation;
    GLfloat b = value;
    hsv_to_rgb_float(&r, &g, &b);
    if(type == CVR_RED)
    {
        drawRect(0.f, 0.f, width(), height(),
                 0.f, g, b,
                 1.f, g, b,
                 1.f, g, b,
                 0.f, g, b);
    }
    else if(type == CVR_GREEN)
    {
        drawRect(0.f, 0.f, width(), height(),
                 r, 0.f, b,
                 r, 1.f, b,
                 r, 1.f, b,
                 r, 0.f, b);
    }
    else if(type == CVR_BLUE)
    {
        drawRect(0.f, 0.f, width(), height(),
                 r, g, 0.f,
                 r, g, 1.f,
                 r, g, 1.f,
                 r, g, 0.f);
    }
    else if(type == CVR_HUE)
    {
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
    else if(type == CVR_ALPHA)
    {
        GLfloat r = hue;
        GLfloat g = saturation;
        GLfloat b = value;
        hsv_to_rgb_float(&r, &g, &b);
        GLfloat val1 = 0.5f;
        GLfloat val2 = 0.25f;
        for(int i = 0; i < width(); i += 7) {
            for(int j = 0; j < height(); j += 7) {
                GLfloat val = ((i + j) % 2 == 0) ? val1 : val2;
                drawSolidRect(i, j, 7, 7, val, val, val,
                              false, false, false, false);
            }
        }
        drawRect(0.f, 0.f, width(), height(),
                 r, g, b, 0.f,
                 r, g, b, 1.f,
                 r, g, b, 1.f,
                 r, g, b, 0.f,
                 false, false, false, false);
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
    MainWindow::getInstance()->callUpdateSchedulers();
}

void ColorValueRect::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::RightButton)
    {
        return;
    }
    emit editingStarted(val);
    mouseInteraction(e->x());
    MainWindow::getInstance()->callUpdateSchedulers();
}

void ColorValueRect::mouseReleaseEvent(QMouseEvent *)
{
    hsl_saturaton_tmp = -1.f;
    emit editingFinished(val);
    MainWindow::getInstance()->callUpdateSchedulers();
}

//void ColorValueRect::wheelEvent(QWheelEvent *e)
//{
//    if(e->delta() > 0) {
//        setValueAndEmitValueChanged(clamp(val + 0.01f, 0.f, 1.f));
//    }
//    else {
//        setValueAndEmitValueChanged(clamp(val - 0.01f, 0.f, 1.f));
//    }
//    MainWindow::getInstance()->callUpdateSchedulers();
//}

void ColorValueRect::mouseInteraction(int x_t)
{
    setValueAndEmitValueChanged(clamp( ((GLfloat)x_t)/width(), 0.f, 1.f));
}

GLfloat ColorValueRect::getVal()
{
    return val;
}

void ColorValueRect::setDisplayedValue(const GLfloat &val_t) {
    val = val_t;
    update();
}

void ColorValueRect::setValueAndEmitValueChanged(GLfloat val_t)
{
    setDisplayedValue(val_t);
    emit valChanged(val);
}
