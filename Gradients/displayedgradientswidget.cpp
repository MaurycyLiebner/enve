#include "displayedgradientswidget.h"
#include <QPainter>
#include "Colors/ColorWidgets/gradientwidget.h"

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
    repaint();
}

void DisplayedGradientsWidget::setNumberGradients(const int &n) {
    setFixedHeight(n*mScrollItemHeight);
}

void DisplayedGradientsWidget::paintGL() {
    mGradientWidget->drawGradients(mDisplayedTop,
                                   mTopGradientId,
                                   mNumberVisibleGradients,
                                   mScrollItemHeight);
    mGradientWidget->drawHoveredGradientBorder(mDisplayedTop,
                                               mTopGradientId,
                                               mHoveredGradientId - mTopGradientId,
                                               mScrollItemHeight);
}

void DisplayedGradientsWidget::mousePressEvent(QMouseEvent *event) {
    int gradientId = event->y()/mScrollItemHeight;
    if(event->button() == Qt::LeftButton) {
        mGradientWidget->gradientLeftPressed(gradientId);
    } else if(event->button() == Qt::RightButton) {
        mGradientWidget->gradientContextMenuReq(gradientId, event->globalPos());
    }
    repaint();
}

void DisplayedGradientsWidget::mouseMoveEvent(QMouseEvent *event) {
    mHoveredGradientId = event->y()/mScrollItemHeight;
    repaint();
}

void DisplayedGradientsWidget::leaveEvent(QEvent *) {
    mHoveredGradientId = -1;
    repaint();
}
