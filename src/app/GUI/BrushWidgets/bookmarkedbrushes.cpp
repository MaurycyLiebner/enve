#include "bookmarkedbrushes.h"
#include "flowlayout.h"

BookmarkedBrushes::BookmarkedBrushes(
        BrushesContext * const brushesContext,
        QWidget * const parent) : QDialog(parent),
    mBrushesContext(brushesContext) {
    setFixedHeight(64);
    setLayout(new QHBoxLayout(this));
    setContentsMargins(0, 0, 0, 0);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->setSpacing(0);
    connect(brushesContext, &BrushesContext::bookmarkAdded,
            this, [this](BrushContexedWrapper* const brush) {
        const auto widget = new BrushWidget(brush, this);
        mWidgets << widget;
        layout()->addWidget(widget);
        updateWidth();
    });
    connect(brushesContext, &BrushesContext::bookmarkRemoved,
            this, [this](BrushContexedWrapper* const brush) {
        for(const auto widget : mWidgets) {
            if(widget->getBrush() == brush) {
                widget->deleteLater();
                mWidgets.removeOne(widget);
                updateWidth();
                break;
            }
        }
    });

    setWindowFlag(Qt::SplashScreen);
}

void BookmarkedBrushes::updateWidth() {
    setFixedWidth(mWidgets.count()*64);
}
