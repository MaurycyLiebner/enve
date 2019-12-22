#include "bookmarkedbrushes.h"
#include "flowlayout.h"

BookmarkedBrushes::BookmarkedBrushes(
        const bool vertical,
        const int dimension,
        BrushesContext * const brushesContext,
        QWidget * const parent) :
    BookmarkedWidget(vertical, dimension, parent),
    mBrushesContext(brushesContext) {
    connect(brushesContext, &BrushesContext::bookmarkAdded,
            this, [this](BrushContexedWrapper* const brush) {
        const auto widget = new BrushWidget(brush, this);
        addWidget(widget);
    });
    connect(brushesContext, &BrushesContext::bookmarkRemoved,
            this, [this](BrushContexedWrapper* const brush) {
        const int iCount = count();
        for(int i = 0 ; i < iCount; i++) {
            const auto widget = static_cast<BrushWidget*>(getWidget(i));
            if(widget->getBrush() == brush) {
                removeWidget(widget);
                break;
            }
        }
    });

}
