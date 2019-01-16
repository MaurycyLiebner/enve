#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <QWidget>
#include <mypaint-brush.h>
#include "smartPointers/sharedpointerdefs.h"
#include "itemwidget.h"
#include "itemwrapper.h"


class BrushWrapper : public ItemWrapper<MyPaintBrush*> {
public:
    BrushWrapper(const QString& name,
                 const QString& collectionName,
                 MyPaintBrush* const brush,
                 const QImage& icon);

    ~BrushWrapper();

    static MyPaintBrush * createBrushDuplicate(
            const BrushWrapper * const src) {
        auto cpy = mypaint_brush_new();
        for(int i = 0; i < MYPAINT_BRUSH_SETTINGS_COUNT; i++) {
            auto id = static_cast<MyPaintBrushSetting>(i);
            auto val = src->getValue(id);
            mypaint_brush_set_base_value(cpy, id, val);
        }
        return cpy;
    }

    static stdsptr<BrushWrapper> createBrushWrapper(
            const QString& fileName, const QString &collectionName);
    void setColor(const float& hue,
                  const float& saturation,
                  const float& value) const {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLOR_H,
                                     hue);
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLOR_S,
                                     saturation);
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLOR_V,
                                     value);
    }

    float getValue(const MyPaintBrushSetting& id) const {
        return mypaint_brush_get_base_value(getItem(), id);
    }

    void setValue(const MyPaintBrushSetting& id, const float& val) const {
        mypaint_brush_set_base_value(getItem(), id, val);
    }

    void setPaintBrushSize(const qreal &size) const {
        auto brush = getItem();
        float brushSize = static_cast<float>(size);
        mypaint_brush_set_base_value(
                    brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC,
                    brushSize);
    }

    void incPaintBrushSize(const qreal &inc) const {
        auto brush = getItem();
        float sizeF = mypaint_brush_get_base_value(
                    brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC);
        setPaintBrushSize(static_cast<qreal>(sizeF) + inc);
    }

    void decPaintBrushSize(const qreal &dec) const {
        auto brush = getItem();
        float sizeF = mypaint_brush_get_base_value(
                    brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC);
        setPaintBrushSize(static_cast<qreal>(sizeF) - dec);
    }

    void startEraseMode() const {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_ERASER,
                                     1.f);
    }

    void finishEraseMode() const {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_ERASER,
                                     0.f);
    }

    void startAlphaLockMode() const {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_LOCK_ALPHA,
                                     1.f);
    }

    void finishAlphaLockMode() const {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_LOCK_ALPHA,
                                     0.f);
    }

    void startColorizeMode() const {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLORIZE,
                                     1.f);
    }

    void finishColorizeMode() const {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLORIZE,
                                     0.f);
    }
};

typedef ItemWidget<BrushWrapper> BrushWidget;

#endif // BRUSHWIDGET_H
