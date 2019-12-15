// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "brushselectionwidget.h"
#include <QDir>
#include <QMenu>
#include "flowlayout.h"
#include "brushwidget.h"
#include <QDebug>
#include <QDockWidget>
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"

bool BrushSelectionWidget::sLoaded = false;
QList<BrushesContext> BrushSelectionWidget::sBrushContexts;

BrushSelectionWidget::BrushSelectionWidget(const int contextId,
                                           QWidget * const parent) :
    QTabWidget(parent), mContextId(contextId) {
    setSizePolicy(QSizePolicy::Preferred, sizePolicy().verticalPolicy());

    updateBrushes();
}

void BrushSelectionWidget::updateBrushes() {
    const auto& context = sGetContext(mContextId);
    for(const auto& coll : context.fCollections) {
        const auto tabScroll = new ScrollArea(this);
        const auto tabWidget = new QWidget(this);
        const auto tabWidgetLay = new FlowLayout(tabWidget, 0, 0, 0);
        for(const auto& brush : coll.fBrushes) {
            const auto bWidget = new BrushWidget(brush.get(), tabWidget);
            connect(bWidget, &BrushWidget::selected,
                    this, &BrushSelectionWidget::brushCWrapperSelected);
            tabWidgetLay->addWidget(bWidget);
        }
        tabWidget->setLayout(tabWidgetLay);
        tabScroll->setWidget(tabWidget);
        addTab(tabScroll, coll.fName);
    }
}

void loadBrushFromFile(const QString &path, BrushCollectionData& coll) {
    QFile dataFile(path);
    if(!dataFile.exists()) return;
    if(!dataFile.open(QIODevice::ReadOnly)) return;
    const QByteArray wholeFile = dataFile.readAll();
    const char *data = wholeFile.constData();
    dataFile.close();

    MyPaintBrush* const brush = mypaint_brush_new();
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

    const QFileInfo fileInfo(dataFile);
    const auto brushName = fileInfo.baseName();
    auto brushWrapper = enve::make_shared<SimpleBrushWrapper>(
                coll.fName, brushName,
                brush, wholeFile);
    coll.fBrushes.append({ brushName, brushWrapper,
                           icon, wholeFile });
}

void loadCollectionFromDir(const QString &mainDirPath,
                           QList<BrushCollectionData> &data) {
    BrushCollectionData collection;
    const QDir brushesDir(mainDirPath);
    collection.fName = brushesDir.dirName();
    const QDir::Filters filter = QDir::NoDotAndDotDot | QDir::AllEntries;
    const QFileInfoList entryList = brushesDir.entryInfoList(filter);
    for(const QFileInfo& fileInfo : entryList) {
        if(fileInfo.isDir()) {
            QString collDirPath = fileInfo.absoluteFilePath();
            loadCollectionFromDir(collDirPath, data);
        } else if(fileInfo.completeSuffix() == "myb") {
            loadBrushFromFile(fileInfo.absoluteFilePath(), collection);
        }
    }
    data << collection;
}

void BrushSelectionWidget::sLoadCollectionsFromDir(const QString &mainDirPath) {
    const QDir brushesDir(mainDirPath);
    if(!brushesDir.exists()) brushesDir.mkpath(mainDirPath);
    const QDir::Filters filter = QDir::NoDotAndDotDot | QDir::AllEntries;
    const QFileInfoList entryList = brushesDir.entryInfoList(filter);
    for(const QFileInfo& fileInfo : entryList) {
        if(fileInfo.isDir()) {
            const QString collName = fileInfo.fileName();
            const QString collDirPath = fileInfo.absoluteFilePath();
            loadCollectionFromDir(collDirPath, BrushCollectionData::sData);
        }
    }
}
