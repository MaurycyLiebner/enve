#include "currentgradientwidget.h"
#include "Colors/ColorWidgets/gradientwidget.h"

CurrentGradientWidget::CurrentGradientWidget(GradientWidget *gradientWidget,
                                             QWidget *parent) :
    GLWidget(parent) {
    mGradientWidget = gradientWidget;
    setFixedHeight(60);
}

void CurrentGradientWidget::paintGL() {
    int halfHeight = height()/2;
    drawMeshBg(0, 0, width(), height());
    mGradientWidget->drawCurrentGradientColors(0, 0, width(), halfHeight);
    mGradientWidget->drawCurrentGradient(0, halfHeight,
                                         width(), height() - halfHeight);
}

void CurrentGradientWidget::mousePressEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        mGradientWidget->colorRightPress(event->x(), event->globalPos());
    } else if(event->button() == Qt::LeftButton) {
        mGradientWidget->colorLeftPress(event->x());
    }
}

void CurrentGradientWidget::mouseMoveEvent(QMouseEvent *event) {
    mGradientWidget->moveColor(event->x());
}
