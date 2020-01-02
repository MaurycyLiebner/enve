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

ImageDataHandler::ImageDataHandler() {}

ImageLoader *ImageDataHandler::scheduleLoad() {
    if(mImage) return nullptr;
    if(mImageLoader) return mImageLoader.get();
    mImageLoader = enve::make_shared<ImageLoader>(mFilePath, this);
    mImageLoader->queTask();
    return mImageLoader.get();
}

ImageLoader::ImageLoader(const QString &filePath,
                         ImageDataHandler * const handler) :
    mTargetHandler(handler), mFilePath(filePath) {

}

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

#include "GUI/edialogs.h"
void ImageFileHandler::replace() {
    const auto importPath = eDialogs::openFile(
                "Change Source", mPath,
                "Image Files (*.png *.jpg)");
    if(!importPath.isEmpty()) setPath(importPath);
}
