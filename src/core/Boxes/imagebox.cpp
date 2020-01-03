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

ImageBox::ImageBox() : BoundingBox(eBoxType::image) {
    prp_setName("Image");
}

ImageBox::ImageBox(const QString &filePath) : ImageBox() {
    setFilePath(filePath);
}

void ImageBox::writeBoundingBox(eWriteStream& dst) {
    BoundingBox::writeBoundingBox(dst);
    dst << mImageFilePath;
}

void ImageBox::readBoundingBox(eReadStream& src) {
    BoundingBox::readBoundingBox(src);
    QString path; src >> path;
    setFilePath(path);
}

void ImageBox::setFilePath(const QString &path) {
    mImageFilePath = path;
    if(mImgCacheHandler) {
        disconnect(mImgCacheHandler, &ImageFileHandler::pathChanged,
                   this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        disconnect(mImgCacheHandler, &ImageFileHandler::reloaded,
                   this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        disconnect(mImgCacheHandler, &ImageFileHandler::deleteApproved,
                   this, &BoundingBox::removeFromParent_k);
    }
    mImgCacheHandler = FilesHandler::sInstance->getFileHandler<ImageFileHandler>(path);

    prp_setName(path.split("/").last());
    if(mImgCacheHandler) {
        connect(mImgCacheHandler, &ImageFileHandler::pathChanged,
                this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        connect(mImgCacheHandler, &ImageFileHandler::reloaded,
                this, &ImageBox::prp_afterWholeInfluenceRangeChanged);
        connect(mImgCacheHandler, &ImageFileHandler::deleteApproved,
                this, &BoundingBox::removeFromParent_k);
    }
    prp_afterWholeInfluenceRangeChanged();
}

void ImageBox::reload() {
    if(mImgCacheHandler) mImgCacheHandler->reloadAction();
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

void ImageBox::changeSourceFile() {
    QString importPath = eDialogs::openFile("Change Source", mImageFilePath,
                                            "Image Files (*.png *.jpg)");
    if(!importPath.isEmpty()) setFilePath(importPath);
}

void ImageBox::setupRenderData(const qreal relFrame,
                               BoxRenderData * const data,
                               Canvas* const scene) {
    BoundingBox::setupRenderData(relFrame, data, scene);
    const auto imgData = static_cast<ImageBoxRenderData*>(data);
    if(mImgCacheHandler->hasImage()) {
        imgData->fImage = mImgCacheHandler->getImageCopy();
    } else {
        const auto loader = mImgCacheHandler->scheduleLoad();
        loader->addDependent(imgData);
    }
}

stdsptr<BoxRenderData> ImageBox::createRenderData() {
    return enve::make_shared<ImageBoxRenderData>(mImgCacheHandler, this);
}

void ImageBoxRenderData::loadImageFromHandler() {
    fImage = fSrcCacheHandler->getImageCopy();
}
