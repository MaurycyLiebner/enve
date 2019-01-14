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
                 MyPaintBrush* brush,
                 const QImage& icon);

    ~BrushWrapper();

    static stdsptr<BrushWrapper> createBrushWrapper(const QString& fileName,
                                                 const QString &collectionName);
    void setColor(const float& hue,
                  const float& saturation,
                  const float& value) {
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

    void setPaintBrushSize(const qreal &size) {
        auto brush = getItem();
        float brushSize = static_cast<float>(size);
        mypaint_brush_set_base_value(
                    brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC,
                    brushSize);
    }

    void incPaintBrushSize(const qreal &inc) {
        auto brush = getItem();
        float sizeF = mypaint_brush_get_base_value(
                    brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC);
        setPaintBrushSize(static_cast<qreal>(sizeF) + inc);
    }

    void decPaintBrushSize(const qreal &dec) {
        auto brush = getItem();
        float sizeF = mypaint_brush_get_base_value(
                    brush, MYPAINT_BRUSH_SETTING_RADIUS_LOGARITHMIC);
        setPaintBrushSize(static_cast<qreal>(sizeF) - dec);
    }

    void startEraseMode() {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_ERASER,
                                     1.f);
    }

    void finishEraseMode() {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_ERASER,
                                     0.f);
    }

    void startAlphaLockMode() {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_LOCK_ALPHA,
                                     1.f);
    }

    void finishAlphaLockMode() {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_LOCK_ALPHA,
                                     0.f);
    }

    void startColorizeMode() {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLORIZE,
                                     1.f);
    }

    void finishColorizeMode() {
        auto brush = getItem();
        mypaint_brush_set_base_value(brush,
                                     MYPAINT_BRUSH_SETTING_COLORIZE,
                                     0.f);
    }
};

typedef ItemWidget<BrushWrapper> BrushWidget;

#endif // BRUSHWIDGET_H
