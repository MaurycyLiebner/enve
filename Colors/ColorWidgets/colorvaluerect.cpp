#include "colorvaluerect.h"
#include "GUI/ColorWidgets/helpers.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include "GUI/mainwindow.h"

ColorValueRect::ColorValueRect(const CVR_TYPE& type_t, QWidget *parent) :
    ColorWidget(parent) {
    mType = type_t;
}

void ColorValueRect::paintGL() {
    GLfloat r = hue;
    GLfloat g = saturation;
    GLfloat b = value;
    hsv_to_rgb_float(&r, &g, &b);
    if(mType == CVR_RED) {
        drawRect(0.f, 0.f, width(), height(),
                 0.f, g, b,
                 1.f, g, b,
                 1.f, g, b,
                 0.f, g, b);
    } else if(mType == CVR_GREEN) {
        drawRect(0.f, 0.f, width(), height(),
                 r, 0.f, b,
                 r, 1.f, b,
                 r, 1.f, b,
                 r, 0.f, b);
    } else if(mType == CVR_BLUE) {
        drawRect(0.f, 0.f, width(), height(),
                 r, g, 0.f,
                 r, g, 1.f,
                 r, g, 1.f,
                 r, g, 0.f);
    } else if(mType == CVR_HUE) {
        GLfloat seg_width = static_cast<GLfloat>(width()/mNumberSegments);
        GLfloat last_r = 0.f;
        GLfloat last_g = 0.f;
        GLfloat last_b = 0.f;
        GLfloat hue_per_i = 1.f/mNumberSegments;
        for(int i = 0; i <= mNumberSegments; i++) {
            GLfloat c_r = i*hue_per_i;
            GLfloat c_g = saturation;
            GLfloat c_b = value;
            hsv_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0) {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == mNumberSegments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    } else if(mType == CVR_HSVSATURATION) {
        GLfloat seg_width = static_cast<GLfloat>(width()/mNumberSegments);
        GLfloat last_r = 0.f;
        GLfloat last_g = 0.f;
        GLfloat last_b = 0.f;
        GLfloat saturation_per_i = 1.f/mNumberSegments;
        for(int i = 0; i <= mNumberSegments; i++) {
            GLfloat c_r = hue;
            GLfloat c_g = i*saturation_per_i;
            GLfloat c_b = value;
            hsv_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0) {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == mNumberSegments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    } else if(mType == CVR_VALUE) {
        GLfloat seg_width = static_cast<GLfloat>(width()/mNumberSegments);
        GLfloat last_r = 0.f;
        GLfloat last_g = 0.f;
        GLfloat last_b = 0.f;
        GLfloat value_per_i = 1.f/mNumberSegments;
        for(int i = 0; i <= mNumberSegments; i++) {
            GLfloat c_r = hue;
            GLfloat c_g = saturation;
            GLfloat c_b = i*value_per_i;
            hsv_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0) {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == mNumberSegments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    } else if(mType == CVR_HSLSATURATION) {
        GLfloat seg_width = static_cast<GLfloat>(width()/mNumberSegments);
        GLfloat last_r = 0.f;
        GLfloat last_g = 0.f;
        GLfloat last_b = 0.f;
        GLfloat saturation_per_i = 1.f/mNumberSegments;

        GLfloat h_t = hue;
        GLfloat s_t = saturation;
        GLfloat l_t = value;
        hsv_to_hsl(&h_t, &s_t, &l_t);
        for(int i = 1; i <= mNumberSegments; i++) {
            GLfloat c_r = hue;
            GLfloat c_g = i*saturation_per_i;
            GLfloat c_b = l_t;
            hsl_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0) {
                drawRect((i - 1)*seg_width, 0.f, seg_width, height(),
                         last_r, last_g, last_b,
                         c_r, c_g, c_b,
                         c_r, c_g, c_b,
                         last_r, last_g, last_b,
                         true, true, i == 1, i == mNumberSegments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    } else if(mType == CVR_LIGHTNESS) {
        GLfloat seg_width = static_cast<GLfloat>(width()/mNumberSegments);
        GLfloat last_r = 0.f;
        GLfloat last_g = 0.f;
        GLfloat last_b = 0.f;
        GLfloat lightness_per_i = 1.f/mNumberSegments;

        GLfloat h_t = hue;
        GLfloat s_t = saturation;
        GLfloat l_t = value;
        hsv_to_hsl(&h_t, &s_t, &l_t);
        for(int i = 0; i <= mNumberSegments; i++) {
            GLfloat c_r = hue;
            GLfloat c_g = s_t;
            GLfloat c_b = i*lightness_per_i;
            hsl_to_rgb_float(&c_r, &c_g, &c_b);
            if(i > 0) {
                drawRect( (i - 1)*seg_width, 0.f, seg_width, height(),
                          last_r, last_g, last_b,
                          c_r, c_g, c_b,
                          c_r, c_g, c_b,
                          last_r, last_g, last_b,
                          true, true, i == 1, i == mNumberSegments);
            }
            last_r = c_r;
            last_g = c_g;
            last_b = c_b;
        }
    } else if(mType == CVR_ALPHA) {
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

    if(shouldValPointerBeLightHSV(hue, saturation, value) ) {
        drawSolidRectCenter(mVal*width(), height()*0.5f, 4.f, height(),
                            1.f, 1.f, 1.f, false, false, false, false);

    } else {
        drawSolidRectCenter(mVal*width(), height()*0.5f, 4.f, height(),
                            0.f, 0.f, 0.f, false, false, false, false);

    }
}

void ColorValueRect::mouseMoveEvent(QMouseEvent *e) {
    mouseInteraction(e->x());
    MainWindow::getInstance()->callUpdateSchedulers();
}

void ColorValueRect::mousePressEvent(QMouseEvent *e) {
    if(e->button() == Qt::RightButton) {
        return;
    }
    emit editingStarted(qVal());
    mouseInteraction(e->x());
    MainWindow::getInstance()->callUpdateSchedulers();
}

void ColorValueRect::mouseReleaseEvent(QMouseEvent *) {
    hsl_saturaton_tmp = -1.f;
    emit editingFinished(qVal());
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

void ColorValueRect::mouseInteraction(const int& x_t) {
    setValueAndEmitValueChanged(
                clamp(static_cast<qreal>(x_t)/width(), 0., 1.));
}


void ColorValueRect::setDisplayedValue(const qreal &val_t) {
    mVal = static_cast<GLfloat>(val_t);
    update();
}

void ColorValueRect::setValueAndEmitValueChanged(const qreal& valT) {
    setDisplayedValue(valT);
    emit valChanged(valT);
}
