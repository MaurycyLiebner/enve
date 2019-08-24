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

#ifndef TMPFILEHANDLERS_H
#define TMPFILEHANDLERS_H
#include "Tasks/updatable.h"
#include <QTemporaryFile>
#include "skia/skiaincludes.h"
class ImageCacheContainer;

class TmpFileDataSaver : public eHddTask {
    e_OBJECT
protected:
    TmpFileDataSaver() {}

    virtual void writeToFile(QIODevice * const file) = 0;
public:
    void process() {
        mTmpFile = qsptr<QTemporaryFile>(new QTemporaryFile());
        if(mTmpFile->open()) {
            writeToFile(mTmpFile.get());
            mTmpFile->close();
        } else {
            RuntimeThrow("Could not open temporary file for writing.");
        }
    }
protected:
    qsptr<QTemporaryFile> mTmpFile;
};

class TmpFileDataLoader : public eHddTask {
    e_OBJECT
protected:
    TmpFileDataLoader(const qsptr<QTemporaryFile> &file) :
        mTmpFile(file) {}

    virtual void readFromFile(QIODevice * const file) = 0;
public:
    void process() {
        if(mTmpFile->open()) {
            readFromFile(mTmpFile.get());
            mTmpFile->close();
        } else {
            RuntimeThrow("Could not open temporary file for reading.");
        }
    }
private:
    const qsptr<QTemporaryFile> mTmpFile;
};

class TmpFileDataDeleter : public eHddTask {
    e_OBJECT
protected:
    TmpFileDataDeleter(const qsptr<QTemporaryFile> &file) :
        mTmpFile(file) {}
public:
    void process() {
        mTmpFile.reset();
    }
private:
    qsptr<QTemporaryFile> mTmpFile;
};

class ImgTmpFileDataSaver : public TmpFileDataSaver {
    e_OBJECT
public:
    typedef std::function<void(qsptr<QTemporaryFile>)> Func;
protected:
    ImgTmpFileDataSaver(const sk_sp<SkImage> &image,
                        const Func& finishedFunc) :
        mImage(image), mFinishedFunc(finishedFunc) {}

    void writeToFile(QIODevice * const file);
    void afterProcessing() {
        if(mFinishedFunc) mFinishedFunc(mTmpFile);
    }
private:
    const sk_sp<SkImage> mImage;
    const Func mFinishedFunc;
};

class ImgTmpFileDataLoader : public TmpFileDataLoader {
    e_OBJECT
public:
    typedef std::function<void(sk_sp<SkImage>)> Func;
protected:
    ImgTmpFileDataLoader(const qsptr<QTemporaryFile> &file,
                         const Func& finishedFunc) :
        TmpFileDataLoader(file), mFinishedFunc(finishedFunc) {}

    void readFromFile(QIODevice * const file);
    void afterProcessing() {
        if(mFinishedFunc) mFinishedFunc(mImage);
    }
private:
    sk_sp<SkImage> mImage;
    const Func mFinishedFunc;
};

#endif // TMPFILEHANDLERS_H
