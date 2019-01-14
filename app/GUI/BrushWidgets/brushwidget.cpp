#include "brushwidget.h"
#include <QFile>
#include <QPainter>
#include <QMouseEvent>
#include <QFileInfo>
#include "smartPointers/sharedpointerdefs.h"

BrushWrapper::BrushWrapper(const QString &name,
                           const QString &collectionName,
                           MyPaintBrush *brush, const QImage &icon)  :
    ItemWrapper<MyPaintBrush*>(name, collectionName,
                               brush, icon) {
}

BrushWrapper::~BrushWrapper() {
    mypaint_brush_unref(getItem());
}

stdsptr<BrushWrapper> BrushWrapper::createBrushWrapper(
        const QString &fileName, const QString& collectionName) {
    QFile dataFile(fileName);
    if(!dataFile.exists()) return nullptr;
    if(dataFile.open(QIODevice::ReadOnly)) {
        QByteArray wholeFile = dataFile.readAll();
        const char *data = wholeFile.constData();
        dataFile.close();

        MyPaintBrush* brush = mypaint_brush_new();
        int result = mypaint_brush_from_string(brush, data);
        if(result != 1) {
            mypaint_brush_unref(brush);
            return nullptr;
        }

        QString iconFileName = fileName;
        iconFileName.replace(".myb", "_prev.png");
        QImage icon;
        if(icon.load(iconFileName)) {
            icon = icon.scaled(64, 64, Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);
        }

        QFileInfo fileInfo(dataFile);
        return SPtrCreate(BrushWrapper)(fileInfo.baseName(),
                                        collectionName, brush, icon);
    }
    return nullptr;
}
