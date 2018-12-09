#include "currentgradientwidget.h"
#include "GUI/GradientWidgets/gradientwidget.h"

CurrentGradientWidget::CurrentGradientWidget(GradientWidget *gradientWidget,
                                             QWidget *parent) :
    GLWidget(parent) {
    setMouseTracking(true);
    mGradientWidget = gradientWidget;
    setFixedHeight(60);
}

void CurrentGradientWidget::paintGL() {

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
