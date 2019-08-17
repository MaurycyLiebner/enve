#include "displayedgradientswidget.h"
#include <QPainter>
#include "GUI/GradientWidgets/gradientwidget.h"
#include "Animators/gradient.h"

DisplayedGradientsWidget::DisplayedGradientsWidget(
                                GradientWidget *gradientWidget,
                                QWidget *parent) :
    GLWidget(parent) {
    setMouseTracking(true);
    mGradientWidget = gradientWidget;
}

void DisplayedGradientsWidget::incTop(const int inc) {
    mDisplayedTop -= inc;
    updateTopGradientId();
}

void DisplayedGradientsWidget::setTop(const int top) {
    mDisplayedTop = top;
    updateTopGradientId();
}

void DisplayedGradientsWidget::updateTopGradientId() {
    int newGradientId = mDisplayedTop/MIN_WIDGET_DIM;
    mHoveredGradientId += newGradientId - mTopGradientId;
    mTopGradientId = newGradientId;
    update();
}

void DisplayedGradientsWidget::setNumberGradients(const int n) {
    setFixedHeight(n*MIN_WIDGET_DIM);
}

#include "GUI/ColorWidgets/colorwidgetshaders.h"
void DisplayedGradientsWidget::paintGL() {
    int nGradients = mGradientWidget->getGradientsCount();
    int visibleGradients = qMin(nGradients, mNumberVisibleGradients);
    int yT = mDisplayedTop;
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(GRADIENT_PROGRAM.fID);
    glBindVertexArray(mPlainSquareVAO);
    Gradient* currentGradient = mGradientWidget->getCurrentGradient();
    for(int i = mTopGradientId; i < mTopGradientId + visibleGradients; i++) {
        int yInverted = height() - yT - MIN_WIDGET_DIM;
        Gradient *gradient = mGradientWidget->getGradientAt(i);
        int nColors = gradient->getColorCount();
        QColor lastColor = gradient->getColorAt(0);
        int xT = 0;
        const float xInc = static_cast<float>(width())/(nColors - 1);
        glUniform2f(GRADIENT_PROGRAM.fMeshSizeLoc,
                    MIN_WIDGET_DIM/(3.f*xInc), 1.f/3);
        for(int j = 1; j < nColors; j++) {
            QColor currentColor = gradient->getColorAt(j);
            glViewport(xT, yInverted, qRound(xInc), MIN_WIDGET_DIM);

            glUniform4f(GRADIENT_PROGRAM.fRGBAColor1Loc,
                        lastColor.redF(), lastColor.greenF(),
                        lastColor.blueF(), lastColor.alphaF());
            glUniform4f(GRADIENT_PROGRAM.fRGBAColor2Loc,
                        currentColor.redF(), currentColor.greenF(),
                        currentColor.blueF(), currentColor.alphaF());

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            if(gradient == currentGradient) {
                glUseProgram(DOUBLE_BORDER_PROGRAM.fID);
                glUniform2f(DOUBLE_BORDER_PROGRAM.fInnerBorderSizeLoc,
                            1.f/xInc, 1.f/MIN_WIDGET_DIM);
                glUniform4f(DOUBLE_BORDER_PROGRAM.fInnerBorderColorLoc,
                            1.f, 1.f, 1.f, 1.f);
                glUniform2f(DOUBLE_BORDER_PROGRAM.fOuterBorderSizeLoc,
                            1.f/xInc, 1.f/MIN_WIDGET_DIM);
                glUniform4f(DOUBLE_BORDER_PROGRAM.fOuterBorderColorLoc,
                            0.f, 0.f, 0.f, 1.f);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glUseProgram(GRADIENT_PROGRAM.fID);
            }
            xT += qRound(xInc);
            lastColor = currentColor;
        }
        if(i == mHoveredGradientId || i == mContextMenuGradientId) {
            glUseProgram(BORDER_PROGRAM.fID);
            glUniform2f(BORDER_PROGRAM.fBorderSizeLoc,
                        1.f/width(), 1.f/MIN_WIDGET_DIM);
            glUniform4f(BORDER_PROGRAM.fBorderColorLoc,
                        1.f, 1.f, 1.f, 1.f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glUseProgram(GRADIENT_PROGRAM.fID);
        }
        yT += MIN_WIDGET_DIM;
    }
}

void DisplayedGradientsWidget::mousePressEvent(QMouseEvent *event) {
    int gradientId = event->y()/MIN_WIDGET_DIM;
    if(event->button() == Qt::LeftButton) {
        mGradientWidget->gradientLeftPressed(gradientId);
    } else if(event->button() == Qt::RightButton) {
        mContextMenuGradientId = gradientId;
        mGradientWidget->gradientContextMenuReq(gradientId, event->globalPos());
        mContextMenuGradientId = -1;
        QPoint relCursorPos = mapFromGlobal(QCursor::pos());
        if(relCursorPos.x() < 0 || relCursorPos.y() < 0 ||
                relCursorPos.x() > width() || relCursorPos.y() > height()) {
            mHoveredGradientId = -1;
        } else {
            mHoveredGradientId = relCursorPos.y()/MIN_WIDGET_DIM;
        }
    }
    update();
}

void DisplayedGradientsWidget::mouseMoveEvent(QMouseEvent *event) {
    mHoveredGradientId = event->y()/MIN_WIDGET_DIM;
    update();
}

void DisplayedGradientsWidget::leaveEvent(QEvent *) {
    mHoveredGradientId = -1;
    update();
}