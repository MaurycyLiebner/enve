#include "brushselectionwidget.h"
#include <QDir>
#include <QMenu>
#include "flowlayout.h"
#include "brushcollection.h"

BrushSelectionWidget::BrushSelectionWidget(QWidget* parent) :
        ItemSelectionWidget<BrushWrapper>(parent) {
    //setDefaultItem(nullptr);
    auto rightPressFunc = [this](ItemSelectionWidgetQObject* selectionWidget,
                                 CollectionAreaQObject*,
                                 StdSelfRef* item,
                                 QPoint globalPos) {
        if(item == nullptr) return;
        QMenu menu(selectionWidget);
        menu.addAction("Bookmark");
        QAction* selectedAction = menu.exec(globalPos);
        if(selectedAction == nullptr) return;
        if(selectedAction->text() == "Bookmark") {
            emit this->brushBookmarked(GetAsPtr(item, BrushWrapper));
        }
    };
    setRightPressedFunction(rightPressFunc);

    QString brushesDir = QDir::homePath() + "/.IsometricEngine/brushes/";
    loadCollectionFromDir(brushesDir);
}

#include <QDebug>
#include <QDockWidget>
void BrushSelectionWidget::brushSelected(BrushWrapper* wrapper) {
    setCurrentItem(wrapper);
}

void BrushSelectionWidget::loadCollectionFromDir(const QString& mainDirPath) {
    QDir brushesDir(mainDirPath);
    if(!brushesDir.exists()) brushesDir.mkpath(mainDirPath);
    QDir::Filters filter = QDir::NoDotAndDotDot | QDir::AllEntries;
    QFileInfoList entryList = brushesDir.entryInfoList(filter);
    for(const QFileInfo& fileInfo : entryList) {
        if(fileInfo.isDir()) {
            QString collName = fileInfo.fileName();
            QString collDirPath = fileInfo.absoluteFilePath();
            BrushCollection *childColl = new BrushCollection(collDirPath, this);
            addChildCollection(childColl, collName);
        }
    }
}
