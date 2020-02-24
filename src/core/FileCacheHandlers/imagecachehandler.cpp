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

#include "imagecachehandler.h"

#include "filecachehandler.h"
#include "Ora/oraimporter.h"
#include "kraimporter.h"

ImageDataHandler::ImageDataHandler() {}

void ImageDataHandler::afterSourceChanged() {
    const QFileInfo info(getFilePath());
    const auto suffix = info.suffix();
    if(suffix == "ora") {
        mType = Type::ora;
    } else if(suffix == "kra") {
        mType = Type::kra;
    } else {
        mType = Type::image;
    }
}

void ImageDataHandler::clearCache() {
    mImage.reset();
    mImageCopies.clear();
    mImageLoader.reset();
}

ImageLoader *ImageDataHandler::scheduleLoad() {
    if(mImage) return nullptr;
    if(mImageLoader) return mImageLoader.get();
    switch(mType) {
    case Type::ora:
        mImageLoader = enve::make_shared<OraLoader>(mFilePath, this);
        break;
    case Type::kra:
        mImageLoader = enve::make_shared<KraLoader>(mFilePath, this);
        break;
    case Type::image:
        mImageLoader = enve::make_shared<ImageLoader>(mFilePath, this);
        break;
    case Type::none: return nullptr;
    }
    if(mImageLoader) mImageLoader->queTask();
    return mImageLoader.get();
}

sk_sp<SkImage> ImageDataHandler::requestCopy(int &stateId) {
    stateId = mStateId;
    if(mImageCopies.isEmpty()) return SkiaHelpers::makeCopy(mImage);
    else return mImageCopies.takeLast();
}

void ImageDataHandler::addImageCopy(const sk_sp<SkImage> &img, const int stateId) {
    if(stateId != mStateId) return;
    mImageCopies << img;
}

void ImageDataHandler::setImage(const sk_sp<SkImage> &img) {
    mImage = img;
    mStateId++;
    mImageCopies.clear();
    mImageLoader.reset();
}

ImageLoader::ImageLoader(const QString &filePath,
                         ImageDataHandler * const handler) :
    mTargetHandler(handler), mFilePath(filePath) {}

void ImageLoader::process() {
    const sk_sp<SkData> data = SkData::MakeFromFileName(
                mFilePath.toUtf8().data());
    mImage = SkImage::MakeFromEncoded(data);
}

void ImageLoader::afterProcessing() {
    mTargetHandler->setImage(mImage);
}

void ImageLoader::afterCanceled() {
    mTargetHandler->setImage(mImage);
}

void OraLoader::process() {
    mImage = ImportORA::loadMergedORAFile(mFilePath, true);
}

void KraLoader::process() {
    mImage = ImportKRA::loadMergedKRAFile(mFilePath, true);
}

#include "GUI/edialogs.h"
#include "filesourcescache.h"
void ImageFileHandler::replace() {
    const QString filters = FileExtensions::imageFilters() +
                            FileExtensions::layersFilters();
    const auto importPath = eDialogs::openFile(
                "Change Source", mPath,
                "Image Files (" + filters + ")");
    if(!importPath.isEmpty()) setPath(importPath);
}
