#include "brushcollection.h"
#include <QDir>
#include <QVBoxLayout>
#include <QScrollBar>
#include "flowlayout.h"
#include "brushselectionwidget.h"

BrushCollection::BrushCollection(const BrushCollectionData &data,
                                 QWidget *parent) :
    CollectionArea<BrushWrapper>("", parent) {
    for(const BrushData& brushD : data.fBrushes) {
        stdsptr<BrushWrapper> itemSptr =
                BrushWrapper::createBrushWrapper(brushD, getName());
        if(!itemSptr) continue;
        auto brushWidget = BrushWidget::createWidget(itemSptr, this);
        if(!brushWidget) continue;
        addWidget(brushWidget);
    }
}
