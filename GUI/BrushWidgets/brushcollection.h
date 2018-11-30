#ifndef BRUSHCOLLECTION_H
#define BRUSHCOLLECTION_H

#include <QScrollArea>
#include "brushwidget.h"
#include "collectionarea.h"
class QVBoxLayout;
class FlowLayout;
struct MyPaintBrush;

class BrushCollection : public CollectionArea<BrushWrapper> {
public:
    BrushCollection(const QString& dirPath,
                    QWidget* parent);

    void loadCollectionFromDir(const QString& mainDirPath);
    bool loadBrushFromFile(const QString& path);
};

#endif // BRUSHCOLLECTION_H
