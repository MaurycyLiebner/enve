#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include "selfref.h"
#include "skqtconversions.h"
#include "skiaincludes.h"
class QTemporaryFile;

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
class CacheContainerTmpFileDataLoader : public Updatable {
public:
    CacheContainerTmpFileDataLoader(const QSharedPointer<QTemporaryFile> &file,
                                    CacheContainer *target);
    void processUpdate();

    void afterUpdate();

    bool isFileUpdatable() { return true; }
    void addSchedulerNow();
private:
    CacheContainer *mTargetCont = NULL;
    sk_sp<SkImage> mImage;
    QSharedPointer<QTemporaryFile> mTmpFile;
};

class CacheContainerTmpFileDataSaver : public Updatable {
public:
    CacheContainerTmpFileDataSaver(const sk_sp<SkImage> &image,
                                   CacheContainer *target);
    void processUpdate();

    void afterUpdate();
    bool isFileUpdatable() { return true; }
    void addSchedulerNow();
private:
    CacheContainer *mTargetCont = NULL;
    sk_sp<SkImage> mImage;
    QSharedPointer<QTemporaryFile> mTmpFile;
};

class CacheContainerTmpFileDataDeleter : public Updatable {
public:
    CacheContainerTmpFileDataDeleter(const QSharedPointer<QTemporaryFile> &file) {
        mTmpFile = file;
    }
    void processUpdate();

    bool isFileUpdatable() { return true; }
    void addSchedulerNow();
private:
    QSharedPointer<QTemporaryFile> mTmpFile;
};

class CacheContainer : public MinimalCacheContainer {
public:
    CacheContainer() {}
    ~CacheContainer();

    Updatable *scheduleLoadFromTmpFile(Updatable *dependent = NULL) {
        if(mSavingToFile) {
            mCancelAfterSaveDataClear = true;
            return mSavingUpdatable;
        }
        if(!mNoDataInMemory || mLoadingFromFile) return NULL;

        mLoadingFromFile = true;
        Updatable *updatable = new CacheContainerTmpFileDataLoader(mTmpFile,
                                                                   this);
        if(dependent != NULL) {
            updatable->addDependent(dependent);
        }
        updatable->addScheduler();
        return updatable;
    }

    void setParentCacheHandler(CacheHandler *handler);
    bool cacheAndFree();
    bool freeAndRemove();
    void setBlocked(const bool &bT);

    int getByteCount() {
        if(mImageSk.get() == NULL) return 0;
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
        if(mTmpFile == NULL) return;
        Updatable *updatable = new CacheContainerTmpFileDataDeleter(mTmpFile);
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
protected:
    int mMemSizeAwaitingSave = 0;
    Updatable *mSavingUpdatable = NULL;
    bool mCancelAfterSaveDataClear = false;
    bool mSavingToFile = false;
    bool mLoadingFromFile = false;
    void saveToTmpFile();
    bool mNoDataInMemory = false;
    QSharedPointer<QTemporaryFile> mTmpFile;

    sk_sp<SkImage> mImageSk;
    CacheHandler *mParentCacheHandler = NULL;
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
