#include "simplebrushwrapper.h"

SimpleBrushWrapper::SimpleBrushWrapper(MyPaintBrush * const brush,
                                         const QByteArray& wholeFile) :
    mBrush(brush), mWholeFile(wholeFile) {}

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
    return SPtrCreate(SimpleBrushWrapper)(brush, mWholeFile);
}
