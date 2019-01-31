#include "brusharraywidget.h"
#include "brushselectionwidget.h"
#include "basicreadwrite.h"

BrushArrayWidget::BrushArrayWidget(
        const Qt::Orientation& orientation,
        QWidget *parent) :
    ArrayWidget(orientation, parent) {}

bool BrushArrayWidget::readBinary(QIODevice *src,
                                  BrushSelectionWidget* brushes) {
    int count = widgetsCount();
    if(src->read(rcChar(&count), sizeof(int)) <= 0) {
        return false;
    }
    for(int i = 0; i < count; i++) {
        QString collectionName;
        QString brushName;
        if(!gRead(src, collectionName)) return false;
        if(!gRead(src, brushName)) return false;
        BrushWrapper* brush = brushes->getItem(collectionName, brushName);
        if(!brush) continue;
        appendBrush(GetAsSPtr(brush, BrushWrapper));
    }
    return true;
}

bool BrushArrayWidget::writeBinary(QIODevice *dst) {
    int count = widgetsCount();
    if(dst->write(rcChar(&count), sizeof(int)) <= 0) {
        return false;
    }
    for(int i = 0; i < count; i++) {
        BrushWidget* wid = static_cast<BrushWidget*>(getWidgetAt(i));
        BrushWrapper* brush = wid->getItem();
        QString collectionName = brush->getCollectionName();
        QString brushName = brush->getName();
        if(!gWrite(dst, collectionName)) {
            return false;
        }
        if(!gWrite(dst, brushName)) {
            return false;
        }
    }
    return true;
}
