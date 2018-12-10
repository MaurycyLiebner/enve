#include "displayedgradientswidget.h"
#include <QPainter>
#include "GUI/GradientWidgets/gradientwidget.h"

DisplayedGradientsWidget::DisplayedGradientsWidget(
                                GradientWidget *gradientWidget,
                                QWidget *parent) :
    GLWidget(parent) {
    setMouseTracking(true);
    mGradientWidget = gradientWidget;
}

void DisplayedGradientsWidget::incTop(const int &inc) {
    mDisplayedTop -= inc;
    updateTopGradientId();
}

void DisplayedGradientsWidget::setTop(const int &top) {
    mDisplayedTop = top;
    updateTopGradientId();
}

void DisplayedGradientsWidget::updateTopGradientId() {
    int newGradientId = mDisplayedTop/mScrollItemHeight;
    mHoveredGradientId += newGradientId - mTopGradientId;
    mTopGradientId = newGradientId;
    update();
}

void DisplayedGradientsWidget::setNumberGradients(const int &n) {
    setFixedHeight(n*mScrollItemHeight);
}
#include "GUI/ColorWidgets/colorwidgetshaders.h"
void DisplayedGradientsWidget::paintGL() {
    int gradListHeight = mScrollItemHeight*mNumberVisibleGradients;

    int yT = mDisplayedTop;
    int nGradients = mGradientWidget->getGradientsCount();
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(GRADIENT_PROGRAM.fID);
    for(int i = nGradients - 1; i >= mTopGradientId; i--) {
        Gradient *gradient = mGradientWidget->getGradientAt(i);
        int nColors = gradient->getColorCount();
        QColor lastColor = gradient->getCurrentColorAt(0);
        qreal xT = 0.;
        qreal xInc = static_cast<qreal>(width())/(nColors - 1);
        glUniform2f(GRADIENT_PROGRAM.fMeshSizeLoc,
                    mScrollItemHeight/(3.f*xInc), 1.f/3);
        for(int j = 1; j < nColors; j++) {
            QColor currentColor = gradient->getCurrentColorAt(j);
            glViewport(qRound(xT), yT, qRound(xInc), mScrollItemHeight);

            glUniform4f(GRADIENT_PROGRAM.fRGBAColor1Loc,
                        lastColor.redF(), lastColor.greenF(),
                        lastColor.blueF(), lastColor.alphaF());
            glUniform4f(GRADIENT_PROGRAM.fRGBAColor2Loc,
                        currentColor.redF(), currentColor.greenF(),
                        currentColor.blueF(), currentColor.alphaF());

            glBindVertexArray(mPlainSquareVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    //        if(gradient == mCurrentGradient) {
    //            GLWidget::drawBorder(0, yT,
    //                       width(), mScrollItemHeight);
    //        }
            xT = qRound(xT) + xInc;
            lastColor = currentColor;
            assertNoGlErrors();
        }
        yT += mScrollItemHeight;
        if(yT > gradListHeight + mDisplayedTop) break;
    }
}

void DisplayedGradientsWidget::mousePressEvent(QMouseEvent *event) {
    int gradientId = event->y()/mScrollItemHeight;
    if(event->button() == Qt::LeftButton) {
        mGradientWidget->gradientLeftPressed(gradientId);
    } else if(event->button() == Qt::RightButton) {
        mGradientWidget->gradientContextMenuReq(gradientId, event->globalPos());
    }
    update();
}

void DisplayedGradientsWidget::mouseMoveEvent(QMouseEvent *event) {
    mHoveredGradientId = event->y()/mScrollItemHeight;
    update();
}

void DisplayedGradientsWidget::leaveEvent(QEvent *) {
    mHoveredGradientId = -1;
    update();
}
