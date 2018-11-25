#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include "selfref.h"
#include "skqtconversions.h"
#include "skiaincludes.h"
#include <QDebug>
#include "sharedpointerdefs.h"
#include "updatable.h"
class QTemporaryFile;
class Canvas;

class CacheHandler;
struct BoundingBoxRenderData;

class MinimalCacheContainer : public StdSelfRef {
public:
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
    MinimalCacheContainer(const bool &addToMemoryHandler = true);

    bool mHandledByMemoryHandler = false;
    bool mBlocked = false;
};
#include "updatable.h"
class CacheContainer;
class CacheContainerTmpFileDataLoader : public _ScheduledExecutor {
public:
    void _processUpdate();

    void afterUpdate();

    bool isFileUpdatable() { return true; }
protected:
    CacheContainerTmpFileDataLoader(const QSharedPointer<QTemporaryFile> &file,
                                    CacheContainer *target);
    void addSchedulerNow();

    const QPointer<CacheContainer> mTargetCont;
    QSharedPointer<QTemporaryFile> mTmpFile;
    sk_sp<SkImage> mImage;
};

class CacheContainerTmpFileDataSaver : public _ScheduledExecutor {
public:
    void _processUpdate();

    void afterUpdate();
    bool isFileUpdatable() { return true; }
protected:
    CacheContainerTmpFileDataSaver(const sk_sp<SkImage> &image,
                                   CacheContainer *target);
    void addSchedulerNow();

    bool mSavingFailed = false;
    const QPointer<CacheContainer> mTargetCont;
    sk_sp<SkImage> mImage;
    QSharedPointer<QTemporaryFile> mTmpFile;
};

class CacheContainerTmpFileDataDeleter : public _ScheduledExecutor {
public:
    void _processUpdate();

    bool isFileUpdatable() { return true; }
protected:
    CacheContainerTmpFileDataDeleter(const QSharedPointer<QTemporaryFile> &file) {
        mTmpFile = file;
    }
    void addSchedulerNow();
    QSharedPointer<QTemporaryFile> mTmpFile;
};

class CacheContainer : public MinimalCacheContainer {
public:
    ~CacheContainer();

    _ScheduledExecutorSPtr scheduleLoadFromTmpFile(
            _ScheduledExecutor* dependent = nullptr);

    bool cacheAndFree();
    bool freeAndRemove();
    void setBlocked(const bool &bT);

    int getByteCount();

    sk_sp<SkImage> getImageSk();

    void scheduleDeleteTmpFile();

    void setDataLoadedFromTmpFile(const sk_sp<SkImage> &img);
    void replaceImageSk(const sk_sp<SkImage> &img);

    const int &getMinRelFrame() const;

    const int &getMaxRelFrame() const;

    void setRelFrame(const int &frame);
    void setMaxRelFrame(const int &maxFrame);
    void setMinRelFrame(const int &minFrame);
    void setRelFrameRange(const int &minFrame, const int &maxFrame);
    bool relFrameInRange(const int &relFrame);
    virtual void drawSk(SkCanvas *canvas, SkPaint *paint = nullptr);
    bool storesDataInMemory();
    void setDataSavedToTmpFile(const QSharedPointer<QTemporaryFile> &tmpFile);
    void setAsCurrentPreviewContainerAfterFinishedLoading(Canvas *canvas);
protected:
    CacheContainer(CacheHandler* parent);

protected:
    void saveToTmpFile();

    bool mCancelAfterSaveDataClear = false;
    bool mSavingToFile = false;
    bool mLoadingFromFile = false;
    bool mNoDataInMemory = false;

    int mMemSizeAwaitingSave = 0;
    int mMinRelFrame = 0;
    int mMaxRelFrame = 0;

    CanvasQPtr mTmpLoadTargetCanvas;
    _ScheduledExecutorSPtr mLoadingUpdatable;
    _ScheduledExecutorSPtr mSavingUpdatable;

    QSharedPointer<QTemporaryFile> mTmpFile;

    sk_sp<SkImage> mImageSk;
    CacheHandler * const mParentCacheHandler_k;
};

class RenderContainer : public CacheContainer {
public:
    RenderContainer() : CacheContainer(nullptr) {}
    virtual ~RenderContainer();

    void drawSk(SkCanvas *canvas, SkPaint *paint = nullptr);

    void updatePaintTransformGivenNewCombinedTransform(
            const QMatrix &combinedTransform);

    void setTransform(const QMatrix &transform);

    const QMatrix &getTransform() const;

    const QMatrix &getPaintTransform() const;

    const SkPoint &getDrawpos() const;

    const qreal &getResolutionFraction() const;

    void setSrcRenderData(BoundingBoxRenderData *data);
    int getRelFrame() {
        return mRelFrame;
    }

    BoundingBoxRenderData *getSrcRenderData() {
        return mSrcRenderData.get();
    }

protected:
    int mRelFrame = 0;
    qreal mResolutionFraction;
    SkPoint mDrawPos;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    BoundingBoxRenderDataSPtr mSrcRenderData;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
