#include "brushselectionwidget.h"
#include <QDir>
#include <QMenu>
#include "flowlayout.h"
#include "brushcollection.h"
#include <QDebug>
#include <QDockWidget>

QList<BrushCollectionData> BrushSelectionWidget::sData;
bool BrushSelectionWidget::sLoaded = false;

BrushSelectionWidget::BrushSelectionWidget(QWidget* parent) :
        ItemSelectionWidget<BrushWrapper>(parent) {
    //setDefaultItem(nullptr);
    auto rightPressFunc = [this](ItemSelectionWidgetQObject* selectionWidget,
                                 CollectionAreaQObject*,
                                 StdSelfRef* item,
                                 QPoint globalPos) {
        if(!item) return;
        QMenu menu(selectionWidget);
        menu.addAction("Bookmark");
        QAction* selectedAction = menu.exec(globalPos);
        if(!selectedAction) return;
        if(selectedAction->text() == "Bookmark") {
            emit this->brushBookmarked(GetAsPtr(item, BrushWrapper));
        }
    };
    setRightPressedFunction(rightPressFunc);

    if(!sLoaded) {
        QString brushesDir = QDir::homePath() + "/.IsometricEngine/brushes/";
        sLoadCollectionsFromDir(brushesDir);
        sLoaded = true;
    }
    for(const BrushCollectionData& coll : sData) {
        addChildCollection(new BrushCollection(coll, this), coll.fName);
    }
    if(mChildCollections.isEmpty()) return;
}

void BrushSelectionWidget::brushSelected(BrushWrapper* wrapper) {
    setCurrentItem(wrapper);
}

void loadBrushFromFile(const QString &path,
                       BrushCollectionData& coll) {
    QFile dataFile(path);
    if(!dataFile.exists()) return;
    if(!dataFile.open(QIODevice::ReadOnly)) return;
    QByteArray wholeFile = dataFile.readAll();
    const char *data = wholeFile.constData();
    dataFile.close();

    MyPaintBrush* brush = mypaint_brush_new();
    if(!mypaint_brush_from_string(brush, data)) {
        mypaint_brush_unref(brush);
        return;
    }

    QString iconFileName = path;
    iconFileName.replace(".myb", "_prev.png");
    QImage icon;
    if(icon.load(iconFileName)) {
        icon = icon.scaled(64, 64, Qt::IgnoreAspectRatio,
                           Qt::SmoothTransformation);
    }

    QFileInfo fileInfo(dataFile);
    auto brushWrapper = SPtrCreate(SimpleBrushWrapper)(brush, wholeFile);
    coll.fBrushes.append({ fileInfo.baseName(), brushWrapper,
                           icon, wholeFile });
}

void loadCollectionFromDir(
        const QString &mainDirPath,
        QList<BrushCollectionData> &data) {
    BrushCollectionData collection;
    QDir brushesDir(mainDirPath);
    collection.fName = brushesDir.dirName();
    QDir::Filters filter = QDir::NoDotAndDotDot | QDir::AllEntries;
    QFileInfoList entryList = brushesDir.entryInfoList(filter);
    for(const QFileInfo& fileInfo : entryList) {
        if(fileInfo.isDir()) {
            QString collDirPath = fileInfo.absoluteFilePath();
            loadCollectionFromDir(collDirPath, data);
        } else if(fileInfo.completeSuffix() == "myb") {
            loadBrushFromFile(fileInfo.absoluteFilePath(),
                              collection);
        }
    }
    data << collection;
}

void BrushSelectionWidget::sLoadCollectionsFromDir(
        const QString &mainDirPath) {
    QDir brushesDir(mainDirPath);
    if(!brushesDir.exists()) brushesDir.mkpath(mainDirPath);
    QDir::Filters filter = QDir::NoDotAndDotDot | QDir::AllEntries;
    QFileInfoList entryList = brushesDir.entryInfoList(filter);
    for(const QFileInfo& fileInfo : entryList) {
        if(fileInfo.isDir()) {
            QString collName = fileInfo.fileName();
            QString collDirPath = fileInfo.absoluteFilePath();
            loadCollectionFromDir(collDirPath, sData);
        }
    }
}
