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

#include "imagesequencecachehandler.h"

#include <QFileDialog>

#include "filesourcescache.h"
#include "fileshandler.h"

ImageSequenceCacheHandler::ImageSequenceCacheHandler() {}

void ImageSequenceCacheHandler::setFolderPath(const QString &folderPath) {
    mFileHandler = FilesHandler::sInstance->getFileHandler<ImageSequenceFileHandler>(folderPath);
}

void ImageSequenceFileHandler::afterPathSet(const QString &folderPath) {
    Q_UNUSED(folderPath);
    reload();
}

sk_sp<SkImage> ImageSequenceFileHandler::getFrameAtFrame(const int relFrame) {
    if(mFrameImageHandlers.isEmpty()) return nullptr;
    const auto cacheHandler = mFrameImageHandlers.at(relFrame);
    if(!cacheHandler) return sk_sp<SkImage>();
    return cacheHandler->getImage();
}

sk_sp<SkImage> ImageSequenceFileHandler::getFrameAtOrBeforeFrame(
        const int relFrame) {
    if(mFrameImageHandlers.isEmpty()) return nullptr;
    if(relFrame >= mFrameImageHandlers.count()) {
        return mFrameImageHandlers.last()->getImage();
    }
    const auto cacheHandler = mFrameImageHandlers.at(relFrame);
    return cacheHandler->getImage();
}

eTask *ImageSequenceFileHandler::scheduleFrameLoad(const int frame) {
    if(mFrameImageHandlers.isEmpty()) return nullptr;
    const auto& imageHandler = mFrameImageHandlers.at(frame);
    if(imageHandler->hasImage()) return nullptr;
    return imageHandler->scheduleLoad();
}

void ImageSequenceFileHandler::reload() {
    mFrameImageHandlers.clear();
    QDir dir(mFolderPath);
    mFileMissing = !dir.exists();
    if(mFileMissing) return;
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name);
    const auto files = dir.entryInfoList();
    for(const auto& fileInfo : files) {
        if(!isImageExt(fileInfo.suffix())) continue;
        const auto filePath = fileInfo.absoluteFilePath();
        const auto handler = ImageDataHandler::sGetCreateDataHandler<ImageDataHandler>(filePath);
        handler->clearCache();
        mFrameImageHandlers << handler;
    }
    if(mFrameImageHandlers.isEmpty()) mFileMissing = true;
}

void ImageSequenceFileHandler::replace(QWidget* const parent) {
    const auto dir = QFileDialog::getExistingDirectory(
                parent, "Import Image Sequence", mPath);
    if(!dir.isEmpty()) setPath(dir);
}
