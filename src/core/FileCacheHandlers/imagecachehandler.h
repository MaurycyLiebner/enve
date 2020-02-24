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

#ifndef IMAGECACHEHANDLER_H
#define IMAGECACHEHANDLER_H
#include "skia/skiahelpers.h"
#include "filecachehandler.h"
#include "Tasks/updatable.h"
class ImageDataHandler;

class ImageLoader : public eHddTask {
    e_OBJECT
protected:
    ImageLoader(const QString &filePath,
                ImageDataHandler * const handler);
public:
    void process();
    void afterProcessing();
    void afterCanceled();
protected:
    ImageDataHandler * const mTargetHandler;
    const QString mFilePath;
    sk_sp<SkImage> mImage;
};

class OraLoader : public ImageLoader {
    e_OBJECT
protected:
    using ImageLoader::ImageLoader;
public:
    void process();
};

class KraLoader : public ImageLoader {
    e_OBJECT
protected:
    using ImageLoader::ImageLoader;
public:
    void process();
};

class ImageDataHandler : public FileDataCacheHandler {
    e_OBJECT
    friend class ImageLoader;

    enum class Type {
        image, kra, ora, none
    };

protected:
    ImageDataHandler();
public:
    void afterSourceChanged();

    void clearCache();

    ImageLoader * scheduleLoad();
    bool hasImage() const { return mImage.get(); }
    sk_sp<SkImage> getImage() const { return mImage; }
    sk_sp<SkImage> requestCopy(int& stateId);
    void addImageCopy(const sk_sp<SkImage>& img, const int stateId);
protected:
    void setImage(const sk_sp<SkImage>& img);
private:
    Type mType = Type::none;
    int mStateId = 0;
    sk_sp<SkImage> mImage;
    QList<sk_sp<SkImage>> mImageCopies;
    stdsptr<ImageLoader> mImageLoader;
};

class ImageFileHandler : public FileCacheHandler {
    e_OBJECT
protected:
    ImageFileHandler() {}

    void afterPathSet(const QString& path) {
        const QFileInfo info(path);
        mFileMissing = !info.exists();
        if(mFileMissing) return mDataHandler.reset();
        mDataHandler = ImageDataHandler::sGetCreateDataHandler<ImageDataHandler>(path);
    }

    void reload() {
        mDataHandler->clearCache();
    }
public:
    void replace();

    ImageLoader * scheduleLoad() {
        if(!mDataHandler) return nullptr;
        return mDataHandler->scheduleLoad();
    }

    bool hasImage() const {
        if(!mDataHandler) return false;
        return mDataHandler->hasImage();
    }

    sk_sp<SkImage> getImage() const {
        if(!mDataHandler) return nullptr;
        return mDataHandler->getImage();
    }

    sk_sp<SkImage> requestImageCopy(int& stateId) const {
        if(!mDataHandler) return nullptr;
        return mDataHandler->requestCopy(stateId);
    }

    void addImageCopy(const sk_sp<SkImage>& img, const int stateId) {
        if(!mDataHandler) return;
        return mDataHandler->addImageCopy(img, stateId);
    }
private:
    qsptr<ImageDataHandler> mDataHandler;
};

#endif // IMAGECACHEHANDLER_H
