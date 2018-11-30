#ifndef BRUSHSELECTIONWIDGET_H
#define BRUSHSELECTIONWIDGET_H

class BrushCollection;
#include <QTabWidget>
#include <mypaint-brush.h>
#include "brushwidget.h"
class FlowLayout;
#include "itemselectionwidget.h"

class BrushSelectionWidget : public ItemSelectionWidget<BrushWrapper> {
    Q_OBJECT
public:
    BrushSelectionWidget(QWidget* parent = nullptr);
    ~BrushSelectionWidget() {}

signals:
    void currentBrushChanged(BrushWrapper*);
    void brushBookmarked(BrushWrapper*);
public slots:
    void brushSelected(BrushWrapper *wrapper);
protected:
    void emitCurrentItemChanged(BrushWrapper* brushWrapper) {
        emit currentBrushChanged(brushWrapper);
    }
private:
    void loadCollectionFromDir(const QString& mainDirPath);
};

#endif // BRUSHSELECTIONWIDGET_H
