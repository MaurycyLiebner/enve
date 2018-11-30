#ifndef BRUSHWIDGET_H
#define BRUSHWIDGET_H

#include <QWidget>
#include <mypaint-brush.h>
#include "selfref.h"
#include "itemwidget.h"
#include "itemwrapper.h"


class BrushWrapper : public ItemWrapper<MyPaintBrush*> {
public:
    BrushWrapper(const QString& name,
                 const QString& collectionName,
                 MyPaintBrush* brush,
                 const QImage& icon)  :
            ItemWrapper<MyPaintBrush*>(name, collectionName,
                                       brush, icon) {
    }

    ~BrushWrapper() {
        mypaint_brush_unref(getItem());
    }

    static sptr<BrushWrapper> createBrushWrapper(const QString& fileName,
                                                 const QString &collectionName);
};

typedef ItemWidget<BrushWrapper> BrushWidget;

#endif // BRUSHWIDGET_H
