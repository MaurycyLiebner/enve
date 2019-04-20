#ifndef TMPFILEHANDLERS_H
#define TMPFILEHANDLERS_H
#include "updatable.h"
#include <QTemporaryFile>
#include "skia/skiaincludes.h"
class ImageCacheContainer;

class TmpFileDataSaver : public _HDDTask {
    friend class StdSelfRef;
protected:
    TmpFileDataSaver();

    virtual void writeToFile(QIODevice * const file) = 0;
public:
    void _processUpdate() {
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

class TmpFileDataLoader : public _HDDTask {
    friend class StdSelfRef;
protected:
    TmpFileDataLoader(const qsptr<QTemporaryFile> &file);

    virtual void readFromFile(QIODevice * const file) = 0;
public:
    void _processUpdate() {
        if(mTmpFile->open()) {
            readFromFile(mTmpFile.get());
            mTmpFile->close();
        } else {
            RuntimeThrow("Could not open temporary file for reading.");
        }
    }
private:
    qsptr<QTemporaryFile> mTmpFile;
};

class TmpFileDataDeleter : public _HDDTask {
    friend class StdSelfRef;
public:
    void _processUpdate() {
        mTmpFile.reset();
    }
protected:
    TmpFileDataDeleter(const qsptr<QTemporaryFile> &file) {
        mTmpFile = file;
    }
    qsptr<QTemporaryFile> mTmpFile;
};

class CacheContainerTmpFileDataSaver : public TmpFileDataSaver {
    friend class StdSelfRef;
public:
    void writeToFile(QIODevice * const file);

    void afterProcessingFinished();
protected:
    CacheContainerTmpFileDataSaver(const sk_sp<SkImage> &image,
                                   ImageCacheContainer *target);

    const stdptr<ImageCacheContainer> mTargetCont;
    sk_sp<SkImage> mImage;
};

class CacheContainerTmpFileDataLoader : public TmpFileDataLoader {
    friend class StdSelfRef;
protected:
    CacheContainerTmpFileDataLoader(const qsptr<QTemporaryFile> &file,
                                    ImageCacheContainer * const target);

    void readFromFile(QIODevice * const file);
    void afterProcessingFinished();
private:
    const stdptr<ImageCacheContainer> mTargetCont;
    sk_sp<SkImage> mImage;
};

#endif // TMPFILEHANDLERS_H
