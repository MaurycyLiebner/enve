#include "gradientslistwidget.h"
#include <QScrollBar>

GradientsListWidget::GradientsListWidget(GradientWidget *gradientWidget,
                                         QWidget *parent) :
    ScrollArea(parent) {
    setStyleSheet("border: 1px solid black;");
    setFixedHeight(6*MIN_WIDGET_HEIGHT);
    verticalScrollBar()->setSingleStep(MIN_WIDGET_HEIGHT);
    mDisplayedGradients = new DisplayedGradientsWidget(gradientWidget, this);
    setWidget(mDisplayedGradients);
}

void GradientsListWidget::scrollContentsBy(int dx, int dy) {
    mDisplayedGradients->incTop(dy);
    QScrollArea::scrollContentsBy(dx, dy);
}
