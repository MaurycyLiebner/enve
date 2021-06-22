// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "externalpaintapphandler.h"

#include "Private/esettings.h"
#include "exceptions.h"
#include "animatedsurface.h"
#include "Ora/oracreator.h"
#include "Ora/oraimporter.h"
#include "Ora/oraparser.h"
#include "Private/document.h"

#include <QProcess>

QSharedPointer<QFileSystemWatcher> ExternalPaintAppHandler::sFileWatcher;
QSharedPointer<QTemporaryDir> ExternalPaintAppHandler::sTmpDir;
ExternalPaintAppHandler::InstanceMap ExternalPaintAppHandler::sInstances;

ExternalPaintAppHandler::ExternalPaintAppHandler(const App app) :
    mApp(app) {
    const auto settings = eSettings::sInstance;
    switch(mApp) {
    case App::gimp:
        mExec = settings->fGimp;
        break;
    case App::mypaint:
        mExec = settings->fMyPaint;
        break;
    case App::krita:
        mExec = settings->fKrita;
        break;
    }
}

ExternalPaintAppHandler::~ExternalPaintAppHandler() {
    removeFromInstanceMap();
}

void ExternalPaintAppHandler::sCreateFileWatcher() {
    const auto fileWatcher = new QFileSystemWatcher();
    sFileWatcher = QSharedPointer<QFileSystemWatcher>(fileWatcher);
}

void ExternalPaintAppHandler::sCreateTmpDir() {
    const auto tmpDir = new QTemporaryDir(QDir::tempPath() + "/enve_paint");
    sTmpDir = QSharedPointer<QTemporaryDir>(tmpDir);
}

void ExternalPaintAppHandler::addToInstanceMap() {
    if(mInMap) return;
    sInstances.insert({mEditSurface.data(), this});
    mInMap = true;
}

void ExternalPaintAppHandler::removeFromInstanceMap() {
    if(!mInMap) return;
    for(auto it = sInstances.begin(); it != sInstances.end(); ++it) {
        if(it->second == this) {
            sInstances.erase(it);
            break;
        }
    }
    mInMap = false;
}

bool ExternalPaintAppHandler::reassingTmpFile() {
    const auto exists = QFile::exists(mEditFilePath);
    if(exists) {
        sFileWatcher->addPath(mEditFilePath);
        mEditFile->setFileName(mEditFilePath);
    }
    return exists;
}

void ExternalPaintAppHandler::setupSuccess() {
    if(!sFileWatcher) sCreateFileWatcher();
    sFileWatcher->addPath(mEditFilePath);
    const auto editFilePath = mEditFilePath;
    connect(this, &QObject::destroyed, sFileWatcher.get(), [editFilePath]() {
        sFileWatcher->removePath(editFilePath);
    });
    connect(sFileWatcher.get(), &QFileSystemWatcher::fileChanged,
            this, [this](const QString& changedPath) {
        if(!mEditSurface) return deleteLater();
        if(mEditFilePath != changedPath) return;
        if(!mEditFile->exists()) {
            if(reassingTmpFile()) {
            // if it is the first time in the row the file is missing
            } else {
                // check again after some time
                QTimer::singleShot(1000, this, [this]() {
                    if(!reassingTmpFile()) deleteLater();
                });
                return;
            }
        }
        const auto oraImg = ImportORA::readOraFileQImage(mEditFilePath);
        bool foundImg = false;
        QImage img;
        bool foundMove = false;
        QPoint move = -mInitial00Pos;
        using Finder = std::function<bool(OraElement&)>;
        const Finder finder = [this, &img, &move, &finder,
                               &foundImg, &foundMove](OraElement& ele) {
            const auto type = ele.fType;
            switch(type) {
            case OraElementType::stack: {
                static_cast<OraStack_Qt&>(ele).findChildren(finder);
            } break;
            case OraElementType::layerPNG: {
                const auto layer = static_cast<OraLayerPNG_Qt&>(ele);
                if(layer.fName == mPivotName) {
                    move -= QPoint(layer.fX, layer.fY);
                    foundMove = true;
                } else if(layer.fName == mLayerName) {
                    move += QPoint(layer.fX, layer.fY);
                    img = layer.fImage;
                    foundImg = true;
                }
            } break;
            default: break;
            }
            return false;
        };
        oraImg->findChildren(finder);
        if(foundImg) {
            mEditSurface->loadPixmap(img);
            if(foundMove) {
                mEditSurface->move(move.x(), move.y());
            } else gPrintException(false, "Could not find the pivot layer");
            mASurface->afterSurfaceChanged(mEditSurface);
        } else {
            gPrintException(false, "Could not find the image layer");
        }
        Document::sInstance->updateScenes();
    });
}

bool ExternalPaintAppHandler::checkAppValid() const {
    const QString check = mExec + " --version";
    const auto checker = new QProcess();
    checker->start(check);
    checker->waitForFinished(3000);
    if(checker->exitStatus() != QProcess::NormalExit ||
       checker->exitCode() != 0) {
        const auto app = appName();
        gPrintException(false, "Could not open " + app + " using \"" + mExec + "\".\n"
                               "Make sure the path to " + app + " executable is valid.");
        return false;
    }
    return true;
}

void ExternalPaintAppHandler::setEditSurface(DrawableAutoTiledSurface* const surf) {
    mEditSurface = surf;
    const auto it = sInstances.find(surf);
    if(it != sInstances.end()) delete it->second;
    addToInstanceMap();
}

void ExternalPaintAppHandler::setupFor(AnimatedSurface * const aSurface,
                                       const QString& layerName) {
    mLayerName = layerName;
    mASurface = aSurface;
    setParent(mASurface);
    setEditSurface(mASurface->getCurrentSurface());
    mInitial00Pos = mEditSurface->zeroTilePos();
    const QString frameStr = QString::number(mASurface->anim_getCurrentRelFrame());
    const auto img = mEditSurface->toImage(true);

    if(!sTmpDir) sCreateTmpDir();
    const QString editFileName = mLayerName + "_" + frameStr + "XXXXXX.ora";

    const auto pathTemplate = sTmpDir->path() + "/" + editFileName;
    mEditFile = qsptr<QTemporaryFile>(new QTemporaryFile(pathTemplate));
    mEditFile->open();
    mEditFilePath = mEditFile->fileName();

    OraImage_Qt oraImg;
    oraImg.fWidth = img.width();
    oraImg.fHeight = img.height();

    const auto oraStack = std::make_shared<OraStack_Qt>();
    oraStack->fName = "Root";
    {
        const auto imgLayer = std::make_shared<OraLayerPNG_Qt>();
        imgLayer->fName = mLayerName;
        imgLayer->fSource = "data/" + mLayerName + ".png";
        imgLayer->fImage = img;
        imgLayer->fSelected = true;
        oraStack->fChildren << imgLayer;
    }
    mPivotName = "Pivot_00";
    {
        const auto pivotLayer = std::make_shared<OraLayerPNG_Qt>();
        pivotLayer->fName = mPivotName;
        pivotLayer->fSource = "data/pivot_00.png";
        pivotLayer->fImage = QImage(5, 5, QImage::Format_RGBA8888_Premultiplied);
        pivotLayer->fImage.fill(Qt::red);
        pivotLayer->fVisible = false;
        pivotLayer->fLocked = true;
        oraStack->fChildren << pivotLayer;
    }
    oraImg.fChildren << oraStack;
    try {
        CreatorOra::save(mEditFile.get(), oraImg, false);
    } catch(...) {
        mEditFile->close();
        RuntimeThrow("Error while saving ora file.");
    }

    mEditFile->close();

    const auto execProc = new QProcess;
    execProc->setProcessChannelMode(QProcess::ForwardedChannels);

    execProc->start(mExec + " \"" + mEditFilePath + "\"");
    connect(execProc, &QProcess::started,
            this, &ExternalPaintAppHandler::setupSuccess);
    connect(execProc, &QProcess::errorOccurred,
            this, [this](const QProcess::ProcessError error) {
        const auto app = appName();
        if(error == QProcess::FailedToStart) {
            gPrintException(false, "Could not open " + app + " using \"" + mExec + "\".\n"
                                   "Make sure the path to " + app + " executable is valid.");
            deleteLater();
        } else gPrintException(false, "Error occurred while running " + app + " using \"" + mExec + "\".\n"
                                      "Make sure the path to " + app + " executable is valid.");
    });
    connect(execProc, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            execProc, &QObject::deleteLater);
}

QString ExternalPaintAppHandler::appName() const {
    switch(mApp) {
    case App::gimp: return "Gimp";
    case App::mypaint: return "MyPaint";
    case App::krita: return "Krita";
    }
    return "invalid";
}
