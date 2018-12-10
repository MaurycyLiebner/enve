#include "currentgradientwidget.h"
#include "GUI/GradientWidgets/gradientwidget.h"
#include "global.h"
#include "GUI/ColorWidgets/colorwidgetshaders.h"

CurrentGradientWidget::CurrentGradientWidget(GradientWidget *gradientWidget,
                                             QWidget *parent) :
    GLWidget(parent) {
    setMouseTracking(true);
    mGradientWidget = gradientWidget;
    setFixedHeight(MIN_WIDGET_HEIGHT);
}

void CurrentGradientWidget::paintGL() {
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(PLAIN_PROGRAM.fID);
    Gradient *gradient = mGradientWidget->getCurrentGradient();
    int nColors = gradient->getColorCount();
    qreal xT = 0.;
    qreal xInc = static_cast<qreal>(width())/nColors;
    glUniform2f(PLAIN_PROGRAM.fMeshSizeLoc,
                height()/(3.f*xInc), 1.f/3);
    for(int j = 0; j < nColors; j++) {
        QColor currentColor = gradient->getCurrentColorAt(j);
        glViewport(qRound(xT), 0, qCeil(xInc), height());


        glUniform4f(PLAIN_PROGRAM.fRGBAColorLoc,
                    currentColor.redF(), currentColor.greenF(),
                    currentColor.blueF(), currentColor.alphaF());

        glBindVertexArray(mPlainSquareVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//        if(gradient == mCurrentGradient) {
//            GLWidget::drawBorder(0, yT,
//                       width(), mScrollItemHeight);
//        }
        xT = qRound(xT) + xInc;
    }
}

void CurrentGradientWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        mGradientWidget->colorRightPress(event->x(), event->globalPos());
    } else if(event->button() == Qt::LeftButton) {
        mGradientWidget->colorLeftPress(event->x());
    }
}

void CurrentGradientWidget::mouseMoveEvent(QMouseEvent *event) {
    if(event->buttons() & Qt::LeftButton) {
        mGradientWidget->moveColor(event->x());
    }
    mHoveredX = event->x();
    update();
}

void CurrentGradientWidget::leaveEvent(QEvent *) {
    mHoveredX = -1;
    update();
}
