#include "brushcollection.h"
#include <QDir>
#include <QVBoxLayout>
#include <QScrollBar>
#include "flowlayout.h"

BrushCollection::BrushCollection(const QString& dirPath,
                                 QWidget *parent) : CollectionArea<BrushWrapper>("", parent) {
    loadCollectionFromDir(dirPath);
}

void BrushCollection::loadCollectionFromDir(const QString &mainDirPath) {
    QDir brushesDir(mainDirPath);
    setName( brushesDir.dirName());
    QDir::Filters filter = QDir::NoDotAndDotDot | QDir::AllEntries;
    QFileInfoList entryList = brushesDir.entryInfoList(filter);
    for(const QFileInfo& fileInfo : entryList) {
        if(fileInfo.isDir()) {
            QString collDirPath = fileInfo.absoluteFilePath();
            loadCollectionFromDir(collDirPath);
        } else if(fileInfo.completeSuffix() == "myb") {
            loadBrushFromFile(fileInfo.absoluteFilePath());
        }
    }
}

bool BrushCollection::loadBrushFromFile(const QString &path) {
    stdsptr<BrushWrapper> itemSptr = BrushWrapper::createBrushWrapper(path, getName());
    if(itemSptr == nullptr) return false;
    BrushWidget* brushWidget = BrushWidget::createWidget(itemSptr, this);
    if(brushWidget == nullptr) return false;
    addWidget(brushWidget);
    return true;
}
