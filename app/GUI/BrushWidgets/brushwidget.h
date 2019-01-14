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
};

typedef ItemWidget<BrushWrapper> BrushWidget;

#endif // BRUSHWIDGET_H
