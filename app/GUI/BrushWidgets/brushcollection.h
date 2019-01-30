#ifndef BRUSHCOLLECTION_H
#define BRUSHCOLLECTION_H

#include <QScrollArea>
#include "brushwidget.h"
#include "collectionarea.h"
class QVBoxLayout;
class FlowLayout;
struct MyPaintBrush;
struct BrushCollectionData;

class BrushCollection : public CollectionArea<BrushWrapper> {
public:
    BrushCollection(const BrushCollectionData& data,
                    QWidget* parent);
};

#endif // BRUSHCOLLECTION_H
