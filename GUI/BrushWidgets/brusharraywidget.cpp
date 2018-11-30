#include "brusharraywidget.h"
#include "brushselectionwidget.h"
#include "qstringio.h"

BrushArrayWidget::BrushArrayWidget(
        const Qt::Orientation& orientation,
        QWidget *parent) :
    ArrayWidget(orientation, parent) {}

bool BrushArrayWidget::readBinary(QIODevice *src,
                                  BrushSelectionWidget* brushes) {
    int count = widgetsCount();
    if(src->read((char*)&count, sizeof(int)) <= 0) {
        return false;
    }
    for(int i = 0; i < count; i++) {
        QString collectionName;
        QString brushName;
        if(!readQString(src, collectionName)) return false;
        if(!readQString(src, brushName)) return false;
        BrushWrapper* brush = brushes->getItem(collectionName, brushName);
        if(brush == nullptr) continue;
        appendBrush(GetAsSPtr(brush, BrushWrapper));
    }
    return true;
}

bool BrushArrayWidget::writeBinary(QIODevice *dst) {
    int count = widgetsCount();
    if(dst->write((char*)&count, sizeof(int)) <= 0) {
        return false;
    }
    for(int i = 0; i < count; i++) {
        BrushWidget* wid = (BrushWidget*)getWidgetAt(i);
        BrushWrapper* brush = wid->getItem();
        QString collectionName = brush->getCollectionName();
        QString brushName = brush->getName();
        if(!writeQString(dst, collectionName)) {
            return false;
        }
        if(!writeQString(dst, brushName)) {
            return false;
        }
    }
    return true;
}
