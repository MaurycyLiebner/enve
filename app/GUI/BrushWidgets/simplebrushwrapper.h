#ifndef SIMPLEBRUSHWRAPPER_H
#define SIMPLEBRUSHWRAPPER_H
#include <mypaint-brush.h>
#include "smartPointers/sharedpointerdefs.h"

class SimpleBrushWrapper : public StdSelfRef {
    friend class StdSelfRef;
public:
    virtual ~SimpleBrushWrapper();

    stdsptr<SimpleBrushWrapper> createDuplicate();
    MyPaintBrush * getBrush() const {
        return mBrush;
    }
    void setColor(const float& hue,
                  const float& saturation,
                  const float& value) const {
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_COLOR_H,
                                     hue);
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_COLOR_S,
                                     saturation);
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_COLOR_V,
                                     value);
    }

    float getValue(const MyPaintBrushSetting& id) const {
        return mypaint_brush_get_base_value(mBrush, id);
    }

    void setValue(const MyPaintBrushSetting& id, const float& val) const {
        mypaint_brush_set_base_value(mBrush, id, val);
    }

    void setPaintBrushSize(const qreal &size) const {
        float brushSize = static_cast<float>(size);
        mypaint_brush_set_base_value(
                    mBrush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC,
                    brushSize);
    }

    void incPaintBrushSize(const qreal &inc) const {
        float sizeF = mypaint_brush_get_base_value(
                    mBrush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC);
        setPaintBrushSize(static_cast<qreal>(sizeF) + inc);
    }

    void decPaintBrushSize(const qreal &dec) const {
        float sizeF = mypaint_brush_get_base_value(
                    mBrush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC);
        setPaintBrushSize(static_cast<qreal>(sizeF) - dec);
    }

    void startEraseMode() const {
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_ERASER,
                                     1);
    }

    void finishEraseMode() const {
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_ERASER,
                                     0);
    }

    void startAlphaLockMode() const {
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_LOCK_ALPHA,
                                     1);
    }

    void finishAlphaLockMode() const {
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_LOCK_ALPHA,
                                     0);
    }

    void startColorizeMode() const {
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_COLORIZE,
                                     1);
    }

    void finishColorizeMode() const {
        mypaint_brush_set_base_value(mBrush,
                                     MYPAINT_BRUSH_SETTING_COLORIZE,
                                     0);
    }

    const QString& getBrushName() const {
        return mBrushName;
    }

    const QString& getCollectionName() const {
        return mCollectionName;
    }
protected:
    SimpleBrushWrapper(const QString& collName,
                       const QString& brushName,
                       MyPaintBrush * const brush,
                       const QByteArray& wholeFile);
private:
    MyPaintBrush * const mBrush;
    const QByteArray mWholeFile;
    const QString mCollectionName;
    const QString mBrushName;
};

#endif // SIMPLEBRUSHWRAPPER_H
