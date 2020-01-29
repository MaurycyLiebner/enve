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

#include "Boxes/imagebox.h"
#include "GUI/edialogs.h"
#include <QMenu>
#include "FileCacheHandlers/imagecachehandler.h"
#include "fileshandler.h"

ImageFileHandler* imageFileHandlerGetter(const QString& path) {
    return FilesHandler::sInstance->getFileHandler<ImageFileHandler>(path);
}

ImageBox::ImageBox() : BoundingBox(eBoxType::image),
    mFileHandler(this,
                 [](const QString& path) {
                     return imageFileHandlerGetter(path);
                 },
                 [this](ImageFileHandler* obj) {
                     return fileHandlerAfterAssigned(obj);
                 },
                 [this](ConnContext& conn, ImageFileHandler* obj) {
                     fileHandlerConnector(conn, obj);
                 }) {
    prp_setName("Image");
}

ImageBox::ImageBox(const QString &filePath) : ImageBox() {
    setFilePath(filePath);
}

void ImageBox::fileHandlerConnector(ConnContext &conn, ImageFileHandler *obj) {
    conn << connect(obj, &ImageFileHandler::pathChanged,
                    this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
    conn << connect(obj, &ImageFileHandler::reloaded,
                    this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
}

void ImageBox::fileHandlerAfterAssigned(ImageFileHandler *obj) {
    Q_UNUSED(obj);
}

void ImageBox::writeBoundingBox(eWriteStream& dst) const {
    BoundingBox::writeBoundingBox(dst);
    dst << mFileHandler.path();
}

void ImageBox::readBoundingBox(eReadStream& src) {
    BoundingBox::readBoundingBox(src);
    QString path; src >> path;
    setFilePath(path);
}

void ImageBox::setFilePath(const QString &path) {
    mFileHandler.assign(path);

    prp_setName(path.split("/").last());
    prp_afterWholeInfluenceRangeChanged();
}

void ImageBox::reload() {
    if(mFileHandler) mFileHandler->reloadAction();
}

#include "typemenu.h"
void ImageBox::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<ImageBox>()) return;
    menu->addedActionsForType<ImageBox>();

    const PropertyMenu::PlainSelectedOp<ImageBox> reloadOp =
    [](ImageBox * box) {
        box->reload();
    };
    menu->addPlainAction("Reload", reloadOp);

    const PropertyMenu::PlainSelectedOp<ImageBox> setSrcOp =
    [](ImageBox * box) {
        box->changeSourceFile();
    };
    menu->addPlainAction("Set Source File...", setSrcOp);

    BoundingBox::setupCanvasMenu(menu);
}
#include "filesourcescache.h"
void ImageBox::changeSourceFile() {
    const QString filters = FileExtensions::imageFilters();
    QString importPath = eDialogs::openFile("Change Source", mFileHandler.path(),
                                            "Image Files (" + filters + ")");
    if(!importPath.isEmpty()) setFilePath(importPath);
}

void ImageBox::setupRenderData(const qreal relFrame,
                               BoxRenderData * const data,
                               Canvas* const scene) {
    BoundingBox::setupRenderData(relFrame, data, scene);
    const auto imgData = static_cast<ImageBoxRenderData*>(data);
    if(mFileHandler->hasImage()) {
        imgData->fImage = mFileHandler->getImageCopy();
    } else {
        const auto loader = mFileHandler->scheduleLoad();
        loader->addDependent(imgData);
    }
}

stdsptr<BoxRenderData> ImageBox::createRenderData() {
    return enve::make_shared<ImageBoxRenderData>(mFileHandler, this);
}

void ImageBoxRenderData::loadImageFromHandler() {
    if(fSrcCacheHandler) fImage = fSrcCacheHandler->getImageCopy();
}
