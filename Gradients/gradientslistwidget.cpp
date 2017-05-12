#include "gradientslistwidget.h"
#include <QScrollBar>

GradientsListWidget::GradientsListWidget(GradientWidget *gradientWidget,
                                         QWidget *parent) :
    ScrollArea(parent) {
    setStyleSheet("border: 1px solid black;");
    setFixedHeight(6*20);
    verticalScrollBar()->setSingleStep(20);
    mDisplayedGradients = new DisplayedGradientsWidget(gradientWidget, this);
    setWidget(mDisplayedGradients);
}

void GradientsListWidget::scrollContentsBy(int dx, int dy) {
    mDisplayedGradients->incTop(dy);
    QScrollArea::scrollContentsBy(dx, dy);
}
