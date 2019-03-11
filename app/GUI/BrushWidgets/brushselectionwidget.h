#ifndef BRUSHSELECTIONWIDGET_H
#define BRUSHSELECTIONWIDGET_H

class BrushCollection;
#include <QTabWidget>
#include <mypaint-brush.h>
#include "brushwidget.h"
class FlowLayout;
#include "itemselectionwidget.h"

struct BrushData {
    QString fName;
    stdsptr<SimpleBrushWrapper> fWrapper;
    QImage fIcon;
    QByteArray fWholeFile;
};

struct BrushCollectionData {
    QString fName;
    QList<BrushData> fBrushes;
};

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
    static void sLoadCollectionsFromDir(const QString& mainDirPath);
    static QList<BrushCollectionData> sData;
    static bool sLoaded;
};

#endif // BRUSHSELECTIONWIDGET_H
