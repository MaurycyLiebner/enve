#include "brushwidget.h"
#include <QFile>
#include <QPainter>
#include <QMouseEvent>
#include <QFileInfo>
#include "smartPointers/sharedpointerdefs.h"
#include "brushselectionwidget.h"

BrushWrapper::BrushWrapper(const QString &name,
                           const QString &collectionName,
                           const stdsptr<SimpleBrushWrapper> &brush,
                           const QImage &icon,
                           const QByteArray &wholeFile)  :
    _SimpleBrushWrapper(brush->getBrush(), wholeFile),
    ItemWrapper<stdsptr<SimpleBrushWrapper>>(
        name, collectionName, brush, icon) {
}

stdsptr<BrushWrapper> BrushWrapper::createBrushWrapper(
        const BrushData& brushD, const QString& collectionName) {
    return SPtrCreate(BrushWrapper)(brushD.fName,
                                    collectionName,
                                    brushD.fWrapper, brushD.fIcon,
                                    brushD.fWholeFile);
}

_SimpleBrushWrapper::_SimpleBrushWrapper(MyPaintBrush * const brush,
                                         const QByteArray& wholeFile) :
    mBrush(brush), mWholeFile(wholeFile) {}

_SimpleBrushWrapper::~_SimpleBrushWrapper() {
    mypaint_brush_unref(mBrush);
}

stdsptr<SimpleBrushWrapper> _SimpleBrushWrapper::createDuplicate() {
    auto brush = mypaint_brush_new();
    const char *data = mWholeFile.constData();

    if(!mypaint_brush_from_string(brush, data)) {
        mypaint_brush_unref(brush);
        return nullptr;
    }
    return SPtrCreate(SimpleBrushWrapper)(brush, mWholeFile);
}

SimpleBrushWrapper::SimpleBrushWrapper(MyPaintBrush * const brush,
                                       const QByteArray& wholeFile) :
    _SimpleBrushWrapper(brush, wholeFile) {}
