#include "brushwidget.h"
#include <QFile>
#include <QPainter>
#include <QMouseEvent>
#include <QFileInfo>
#include "sharedpointerdefs.h"

stdsptr<BrushWrapper> BrushWrapper::createBrushWrapper(const QString &fileName,
                                                    const QString& collectionName) {
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
