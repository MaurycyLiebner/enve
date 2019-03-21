#include "simplebrushwrapper.h"

SimpleBrushWrapper::SimpleBrushWrapper(const QString &collName,
                                       const QString &brushName,
                                       MyPaintBrush * const brush,
                                       const QByteArray& wholeFile) :
    mBrush(brush), mWholeFile(wholeFile),
    mCollectionName(collName), mBrushName(brushName) {
}

SimpleBrushWrapper::~SimpleBrushWrapper() {
    mypaint_brush_unref(mBrush);
}

stdsptr<SimpleBrushWrapper> SimpleBrushWrapper::createDuplicate() {
    auto brush = mypaint_brush_new();
    const char *data = mWholeFile.constData();

    if(!mypaint_brush_from_string(brush, data)) {
        mypaint_brush_unref(brush);
        return nullptr;
    }
    return SPtrCreate(SimpleBrushWrapper)(
                mCollectionName, mBrushName,
                brush, mWholeFile);
}
