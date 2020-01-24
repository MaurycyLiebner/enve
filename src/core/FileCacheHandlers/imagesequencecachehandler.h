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

#ifndef IMAGESEQUENCECACHEHANDLER_H
#define IMAGESEQUENCECACHEHANDLER_H
#include "imagecachehandler.h"
#include "animationcachehandler.h"

class ImageSequenceFileHandler : public FileCacheHandler {
protected:
    void afterPathSet(const QString& folderPath);
    void reload();
public:
    void replace();

    sk_sp<SkImage> getFrameAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame);
    eTask* scheduleFrameLoad(const int frame);
    int getFrameCount() const { return mFrameImageHandlers.count(); }
private:
    QList<qsptr<ImageDataHandler>> mFrameImageHandlers;
};

class ImageSequenceCacheHandler : public AnimationFrameHandler {
    e_OBJECT
protected:
    ImageSequenceCacheHandler(ImageSequenceFileHandler* fileHandler);
public:
    sk_sp<SkImage> getFrameAtFrame(const int relFrame) {
        if(!mFileHandler) return nullptr;
        return mFileHandler->getFrameAtFrame(relFrame);
    }
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame) {
        if(!mFileHandler) return nullptr;
        return mFileHandler->getFrameAtOrBeforeFrame(relFrame);
    }
    eTask* scheduleFrameLoad(const int frame) {
        if(!mFileHandler) return nullptr;
        return mFileHandler->scheduleFrameLoad(frame);
    }
    void reload() {
        if(mFileHandler) mFileHandler->reloadAction();
    }
    int getFrameCount() const {
        if(!mFileHandler) return 0;
        return mFileHandler->getFrameCount();
    }
private:
    const qptr<ImageSequenceFileHandler> mFileHandler;

};
#endif // IMAGESEQUENCECACHEHANDLER_H
