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

#include "imagesequencebox.h"
#include "FileCacheHandlers/imagesequencecachehandler.h"
#include "filesourcescache.h"
#include "GUI/edialogs.h"
#include "fileshandler.h"
#include "Boxes/containerbox.h"

ImageSequenceFileHandler* imageSequenceFileHandlerGetter(const QString& path) {
    const auto fileHandler = FilesHandler::sInstance;
    return fileHandler->getFileHandler<ImageSequenceFileHandler>(path);
}

ImageSequenceBox::ImageSequenceBox() :
    AnimationBox(eBoxType::imageSequence),
    mFileHandler(this,
                 [](const QString& path) {
                     return imageSequenceFileHandlerGetter(path);
                 },
                 [this](ImageSequenceFileHandler* obj) {
                     return fileHandlerAfterAssigned(obj);
                 },
                 [this](ConnContext& conn, ImageSequenceFileHandler* obj) {
                     fileHandlerConnector(conn, obj);
                 }) {
    prp_setName("Image Sequence");
}

void ImageSequenceBox::fileHandlerConnector(ConnContext &conn,
                                            ImageSequenceFileHandler *obj) {
    Q_UNUSED(conn)
    Q_UNUSED(obj)
}

void ImageSequenceBox::fileHandlerAfterAssigned(ImageSequenceFileHandler *obj) {
    if(obj) {
        mSrcFramesCache = enve::make_shared<ImageSequenceCacheHandler>(obj);
    } else mSrcFramesCache.reset();
    animationDataChanged();
}

void ImageSequenceBox::setFolderPath(const QString &folderPath) {
    mFileHandler.assign(folderPath);
}

void ImageSequenceBox::changeSourceFile() {
    const auto dir = eDialogs::openDir("Import Image Sequence",
                                       mFileHandler.path());
    if(!dir.isEmpty()) setFolderPath(dir);
}

void ImageSequenceBox::writeBoundingBox(eWriteStream& dst) const {
    AnimationBox::writeBoundingBox(dst);
    dst << mFileHandler.path();
}

void ImageSequenceBox::readBoundingBox(eReadStream& src) {
    AnimationBox::readBoundingBox(src);
    QString dir; src >> dir;
    setFolderPath(dir);
}
