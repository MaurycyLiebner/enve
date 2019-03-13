#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <QWidget>
#include <mypaint-brush.h>
#include "smartPointers/sharedpointerdefs.h"
#include "itemwidget.h"
#include "itemwrapper.h"
class SimpleBrushWrapper;
struct BrushData;

class _SimpleBrushWrapper {
public:
    virtual ~_SimpleBrushWrapper();

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
protected:
    _SimpleBrushWrapper(MyPaintBrush * const brush,
                        const QByteArray& wholeFile);
private:
    MyPaintBrush *mBrush;
    QByteArray mWholeFile;
};

class SimpleBrushWrapper :
        public _SimpleBrushWrapper,
        public StdSelfRef {
    friend class StdSelfRef;
protected:
    SimpleBrushWrapper(MyPaintBrush * const brush,
                       const QByteArray &wholeFile);
};

class BrushWrapper : public _SimpleBrushWrapper,
        public ItemWrapper<stdsptr<SimpleBrushWrapper>> {
    friend class StdSelfRef;
public:
    static stdsptr<BrushWrapper> createBrushWrapper(
            const BrushData &brushD, const QString &collectionName);
protected:
    BrushWrapper(const QString& name,
                 const QString& collectionName,
                 const stdsptr<SimpleBrushWrapper>& brush,
                 const QImage& icon,
                 const QByteArray &wholeFile);
private:
    stdsptr<SimpleBrushWrapper> mTargetBrush;
};

typedef ItemWidget<BrushWrapper> BrushWidget;

#endif // BRUSHWIDGET_H
