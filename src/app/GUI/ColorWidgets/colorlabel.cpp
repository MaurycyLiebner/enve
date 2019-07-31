#include "colorlabel.h"
#include "colorhelpers.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWidget>
#include "colorwidgetshaders.h"

ColorLabel::ColorLabel(QWidget *parent) : ColorWidget(parent) {
    //setMinimumSize(80, 20);
    //setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void ColorLabel::mousePressEvent(QMouseEvent *e) {
    Q_UNUSED(e);
}

void ColorLabel::setAlpha(const qreal alpha_t) {
    mAlpha = alpha_t;
    update();
}

void ColorLabel::paintGL() {
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(PLAIN_PROGRAM.fID);
    float r = mHue;
    float g = mSaturation;
    float b = mValue;
    hsv_to_rgb_float(r, g, b);

    int halfWidth = width()/2;
    glViewport(0, 0, halfWidth, height());
    glUniform4f(PLAIN_PROGRAM.fRGBAColorLoc, r, g, b,
                static_cast<float>(mAlpha));
    glUniform2f(PLAIN_PROGRAM.fMeshSizeLoc, height()/(1.5f*width()), 1.f/3);
    glBindVertexArray(mPlainSquareVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


    glViewport(halfWidth, 0, width() - halfWidth, height());
    glUniform4f(PLAIN_PROGRAM.fRGBAColorLoc, r, g, b, 1.f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
