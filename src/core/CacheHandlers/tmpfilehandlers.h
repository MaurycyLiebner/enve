#ifndef TMPFILEHANDLERS_H
#define TMPFILEHANDLERS_H
#include "Tasks/updatable.h"
#include <QTemporaryFile>
#include "skia/skiaincludes.h"
class ImageCacheContainer;

class TmpFileDataSaver : public HDDTask {
    friend class StdSelfRef;
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

class TmpFileDataLoader : public HDDTask {
    friend class StdSelfRef;
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

class TmpFileDataDeleter : public HDDTask {
    friend class StdSelfRef;
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
    friend class StdSelfRef;
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
    friend class StdSelfRef;
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
