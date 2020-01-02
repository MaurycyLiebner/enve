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

ImageSequenceBox::ImageSequenceBox() :
    AnimationBox(eBoxType::imageSequence) {
    prp_setName("Image Sequence");
}

void ImageSequenceBox::setFolderPath(const QString &folderPath) {
    const auto iscHandler = enve::make_shared<ImageSequenceCacheHandler>();
    iscHandler->setFolderPath(folderPath);
    mSrcFramesCache = iscHandler;
    mDirPath = folderPath;
    animationDataChanged();
}

void ImageSequenceBox::changeSourceFile() {
    const auto dir = eDialogs::openDir("Import Image Sequence", mDirPath);
    if(!dir.isEmpty()) setFolderPath(dir);
}

void ImageSequenceBox::writeBoundingBox(eWriteStream& dst) {
    AnimationBox::writeBoundingBox(dst);
    dst << mDirPath;
}

void ImageSequenceBox::readBoundingBox(eReadStream& src) {
    AnimationBox::readBoundingBox(src);
    QString dir; src >> dir;
    setFolderPath(dir);
}
