#include "brushselectionscrollarea.h"
#include "brushselectionwidget.h"

BrushSelectionScrollArea::BrushSelectionScrollArea(QWidget *parent) :
    ScrollArea(parent) {
    mBrushesWidget = new BrushSelectionWidget(this);
    setWidget(mBrushesWidget);
    connect(mBrushesWidget, SIGNAL(brushSelected(const Brush*)),
            this, SIGNAL(brushSelected(const Brush*)));
    connect(mBrushesWidget, SIGNAL(brushReplaced(const Brush*, const Brush*)),
            this, SIGNAL(brushReplaced(const Brush*, const Brush*)));
    setMinimumSize(100, 100);
}

const Brush *BrushSelectionScrollArea::getCurrentBrush() {
    return mBrushesWidget->getCurrentBrush();
}

void BrushSelectionScrollArea::setCurrentBrush(const Brush *brush) {
    mBrushesWidget->setCurrentBrush(brush);
}

void BrushSelectionScrollArea::saveBrushesForProject(QIODevice *target) {
    mBrushesWidget->saveBrushesForProject(target);
}

void BrushSelectionScrollArea::readBrushesForProject(QIODevice *target) {
    mBrushesWidget->readBrushesForProject(target);
}
