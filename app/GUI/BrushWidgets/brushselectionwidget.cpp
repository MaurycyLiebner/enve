#include "brushselectionwidget.h"
#include <QDir>
#include <QMenu>
#include "flowlayout.h"
#include "brushwidget.h"
#include <QDebug>
#include <QDockWidget>

QList<BrushCollectionData> BrushSelectionWidget::sData;
bool BrushSelectionWidget::sLoaded = false;
QList<BrushesContext> BrushSelectionWidget::sBrushContexts;

BrushSelectionWidget::BrushSelectionWidget(const int &contextId,
                                           QWidget * const parent) :
    QTabWidget(parent), mContextId(contextId) {
    setSizePolicy(QSizePolicy::Preferred, sizePolicy().verticalPolicy());

    updateBrushes();
}

void BrushSelectionWidget::updateBrushes() {
    const auto& context = sGetContext(mContextId);
    for(const auto& coll : context.fCollections) {
        const auto tabWidget = new QWidget(this);
        const auto tabWidgetLay = new FlowLayout(tabWidget);
        for(const auto& brush : coll.fBrushes) {
            const auto bWidget = new BrushWidget(brush.get(), tabWidget);
            connect(bWidget, &BrushWidget::selected,
                    this, &BrushSelectionWidget::brushCWrapperSelected);
            tabWidgetLay->addWidget(bWidget);
        }
        tabWidget->setLayout(tabWidgetLay);
        addTab(tabWidget, coll.fName);
    }
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
