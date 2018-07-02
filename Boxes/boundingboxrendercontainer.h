#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include "selfref.h"
#include "skqtconversions.h"
#include "skiaincludes.h"
#include <QDebug>
class QTemporaryFile;
class Canvas;

class CacheHandler;
struct BoundingBoxRenderData;

class MinimalCacheContainer : public StdSelfRef {
public:
    MinimalCacheContainer(const bool &addToMemoryHandler = true);

    virtual ~MinimalCacheContainer();

    bool cacheFreeAndRemoveFromMemoryHandler();

    virtual bool cacheAndFree() = 0;
    virtual bool freeAndRemove() = 0;

    virtual int getByteCount() = 0;

    virtual void setBlocked(const bool &bT) {
        mBlocked = bT;
    }

    void setHandledByMemoryHanlder(const bool &bT) {
        mHandledByMemoryHandler = bT;
    }

    const bool &handledByMemoryHandler() {
        return mHandledByMemoryHandler;
    }
protected:
    bool mHandledByMemoryHandler = false;
    bool mBlocked = false;
};
#include "updatable.h"
class CacheContainer;
class CacheContainerTmpFileDataLoader : public _ScheduledExecutor {
public:
    CacheContainerTmpFileDataLoader(const QSharedPointer<QTemporaryFile> &file,
                                    CacheContainer *target);
    void _processUpdate();

    void afterUpdate();

    bool isFileUpdatable() { return true; }
protected:
    void addSchedulerNow();
    CacheContainer *mTargetCont = nullptr;
    sk_sp<SkImage> mImage;
    QSharedPointer<QTemporaryFile> mTmpFile;
};

class CacheContainerTmpFileDataSaver : public _ScheduledExecutor {
public:
    CacheContainerTmpFileDataSaver(const sk_sp<SkImage> &image,
                                   CacheContainer *target);
    void _processUpdate();

    void afterUpdate();
    bool isFileUpdatable() { return true; }
protected:
    bool mSavingFailed = false;
    void addSchedulerNow();
    CacheContainer *mTargetCont = nullptr;
    sk_sp<SkImage> mImage;
    QSharedPointer<QTemporaryFile> mTmpFile;
};

class CacheContainerTmpFileDataDeleter : public _ScheduledExecutor {
public:
    CacheContainerTmpFileDataDeleter(const QSharedPointer<QTemporaryFile> &file) {
        mTmpFile = file;
    }
    void _processUpdate();

    bool isFileUpdatable() { return true; }
protected:
    void addSchedulerNow();
    QSharedPointer<QTemporaryFile> mTmpFile;
};

class CacheContainer : public MinimalCacheContainer {
public:
    CacheContainer() {}
    ~CacheContainer();

    _ScheduledExecutor *scheduleLoadFromTmpFile(_ScheduledExecutor *dependent = nullptr) {
        if(mSavingToFile) {
            mCancelAfterSaveDataClear = true;
            return mSavingUpdatable;
        }
        if(!mNoDataInMemory) return nullptr;
        if(mLoadingFromFile) return mLoadingUpdatable;

        mLoadingFromFile = true;
        mLoadingUpdatable = new CacheContainerTmpFileDataLoader(mTmpFile,
                                                                this);
        if(dependent != nullptr) {
            mLoadingUpdatable->addDependent(dependent);
        }
        mLoadingUpdatable->addScheduler();
        return mLoadingUpdatable;
    }

    void setParentCacheHandler(CacheHandler *handler);
    bool cacheAndFree();
    bool freeAndRemove();
    void setBlocked(const bool &bT);

    int getByteCount() {
        if(mImageSk.get() == nullptr) return 0;
        SkPixmap pixmap;
        if(mImageSk->peekPixels(&pixmap)) {
            return pixmap.width()*pixmap.height()*
                    pixmap.info().bytesPerPixel();
        }
        return 0;
    }

    sk_sp<SkImage> getImageSk() {
        return mImageSk;
    }

    void scheduleDeleteTmpFile() {
        if(mTmpFile == nullptr) return;
        _ScheduledExecutor *updatable = new CacheContainerTmpFileDataDeleter(mTmpFile);
        mTmpFile.reset();
        updatable->addScheduler();
    }

    void setDataLoadedFromTmpFile(const sk_sp<SkImage> &img);
    void replaceImageSk(const sk_sp<SkImage> &img);

    const int &getMinRelFrame() const;

    const int &getMaxRelFrame() const;

    void setRelFrame(const int &frame);
    void setMaxRelFrame(const int &maxFrame);
    void setMinRelFrame(const int &minFrame);
    void setRelFrameRange(const int &minFrame, const int &maxFrame);
    bool relFrameInRange(const int &relFrame);
    virtual void drawSk(SkCanvas *canvas);
    bool storesDataInMemory() {
        return !mNoDataInMemory;
    }
    void setDataSavedToTmpFile(const QSharedPointer<QTemporaryFile> &tmpFile);
    void setAsCurrentPreviewContainerAfterFinishedLoading(Canvas *canvas) {
        mTmpLoadTargetCanvas = canvas;
    }
protected:
    Canvas *mTmpLoadTargetCanvas = nullptr;
    int mMemSizeAwaitingSave = 0;
    _ScheduledExecutor *mLoadingUpdatable = nullptr;
    _ScheduledExecutor *mSavingUpdatable = nullptr;
    bool mCancelAfterSaveDataClear = false;
    bool mSavingToFile = false;
    bool mLoadingFromFile = false;
    void saveToTmpFile();
    bool mNoDataInMemory = false;
    QSharedPointer<QTemporaryFile> mTmpFile;

    sk_sp<SkImage> mImageSk;
    CacheHandler *mParentCacheHandler = nullptr;
    int mMinRelFrame = 0;
    int mMaxRelFrame = 0;
};

class RenderContainer : public CacheContainer {
public:
    RenderContainer() {}
    virtual ~RenderContainer();

    void drawSk(SkCanvas *canvas, SkPaint *paint);

    void updatePaintTransformGivenNewCombinedTransform(
            const QMatrix &combinedTransform);

    void setTransform(const QMatrix &transform);

    const QMatrix &getTransform() const;

    const QMatrix &getPaintTransform() const;

    const SkPoint &getDrawpos() const;

    const qreal &getResolutionFraction() const;

    void setVariablesFromRenderData(BoundingBoxRenderData *data);
    int getRelFrame() {
        return mRelFrame;
    }

    BoundingBoxRenderData *getSrcRenderData() {
        return mSrcRenderData.get();
    }

protected:
    std::shared_ptr<BoundingBoxRenderData> mSrcRenderData;
    int mRelFrame = 0;
    qreal mResolutionFraction;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    SkPoint mDrawPos;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
