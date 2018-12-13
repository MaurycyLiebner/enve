#include "colorvaluerect.h"
#include "GUI/ColorWidgets/helpers.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPainter>
#include "GUI/mainwindow.h"
#include "colorwidgetshaders.h"
#include "global.h"

ColorValueRect::ColorValueRect(const CVR_TYPE& type_t, QWidget *parent) :
    ColorWidget(parent) {
    mType = type_t;
}

void ColorValueRect::paintGL() {
    assertNoGlErrors();
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    assertNoGlErrors();
    ColorProgram programToUse;
    if(mType == CVR_RED) {
        programToUse = RED_PROGRAM;
    } else if(mType == CVR_GREEN) {
        programToUse = GREEN_PROGRAM;
    } else if(mType == CVR_BLUE) {
        programToUse = BLUE_PROGRAM;
    } else if(mType == CVR_HUE) {
        programToUse = HUE_PROGRAM;
    } else if(mType == CVR_HSVSATURATION) {
        programToUse = HSV_SATURATION_PROGRAM;
    } else if(mType == CVR_VALUE) {
        programToUse = VALUE_PROGRAM;
    } else if(mType == CVR_HSLSATURATION) {
        programToUse = HSL_SATURATION_PROGRAM;
    } else if(mType == CVR_LIGHTNESS) {
        programToUse = LIGHTNESS_PROGRAM;
    } else if(mType == CVR_ALPHA) {
        programToUse = ALPHA_PROGRAM;
    } else {
        assert(false);
    }
    assertNoGlErrors();
    glUseProgram(programToUse.fID);
    assertNoGlErrors();
    if(programToUse.fHSVColorLoc >= 0) {
        glUniform3f(programToUse.fHSVColorLoc,
                    mHue, mSaturation, mValue);
        assertNoGlErrors();
    }
    if(programToUse.fRGBColorLoc >= 0) {
        float r = mHue;
        float g = mSaturation;
        float b = mValue;
        hsv_to_rgb_float(r, g, b);
        glUniform3f(programToUse.fRGBColorLoc, r, g, b);
        assertNoGlErrors();
    }
    if(programToUse.fHSLColorLoc >= 0) {
        float h = mHue;
        float s = mSaturation;
        float l = mValue;
        hsv_to_hsl(h, s, l);
        glUniform3f(programToUse.fHSLColorLoc, h, s, l);
        assertNoGlErrors();
    }
    glUniform1f(programToUse.fCurrentValueLoc,
                mVal);
    assertNoGlErrors();
    glUniform1f(programToUse.fHandleWidthLoc,
                2.f/width());
    assertNoGlErrors();
    glUniform1i(programToUse.fLightHandleLoc,
                shouldValPointerBeLightHSV(mHue, mSaturation, mValue));
    assertNoGlErrors();
    if(programToUse.fMeshSizeLoc >= 0) {
        glUniform2f(programToUse.fMeshSizeLoc,
                    height()/(3.f*width()), 1.f/3);
        assertNoGlErrors();
    }
    glBindVertexArray(mPlainSquareVAO);
    assertNoGlErrors();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    assertNoGlErrors();
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
    mHslSaturatonTmp = -1.f;
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
