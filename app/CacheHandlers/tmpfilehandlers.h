#ifndef TMPFILEHANDLERS_H
#define TMPFILEHANDLERS_H
#include "updatable.h"
#include <QTemporaryFile>
#include "skia/skiaincludes.h"
class ImageCacheContainer;
class CacheContainerTmpFileDataLoader : public _HDDTask {
    friend class StdSelfRef;
public:
    void _processUpdate();

    void afterProcessingFinished();
protected:
    CacheContainerTmpFileDataLoader(const qsptr<QTemporaryFile> &file,
                                    ImageCacheContainer *target);

    const stdptr<ImageCacheContainer> mTargetCont;
    qsptr<QTemporaryFile> mTmpFile;
    sk_sp<SkImage> mImage;
};

class CacheContainerTmpFileDataSaver : public _HDDTask {
    friend class StdSelfRef;
public:
    void _processUpdate();

    void afterProcessingFinished();
protected:
    CacheContainerTmpFileDataSaver(const sk_sp<SkImage> &image,
                                   ImageCacheContainer *target);

    bool mSavingFailed = false;
    const stdptr<ImageCacheContainer> mTargetCont;
    sk_sp<SkImage> mImage;
    qsptr<QTemporaryFile> mTmpFile;
};

class CacheContainerTmpFileDataDeleter : public _HDDTask {
    friend class StdSelfRef;
public:
    void _processUpdate();
protected:
    CacheContainerTmpFileDataDeleter(const qsptr<QTemporaryFile> &file) {
        mTmpFile = file;
    }
    qsptr<QTemporaryFile> mTmpFile;
};
#endif // TMPFILEHANDLERS_H
