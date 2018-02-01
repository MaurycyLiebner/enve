#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include <unordered_map>
#include <QString>
#include <QList>
#include "skiaincludes.h"
#include "Boxes/rendercachehandler.h"
#include "updatable.h"
class BoundingBox;
typedef QWeakPointer<BoundingBox> BoundingBoxQWPtr;
class FileSourceListVisibleWidget;

extern bool isVideoExt(const QString &extension);
extern bool isSoundExt(const QString &extension);
extern bool isVectorExt(const QString &extension);
extern bool isImageExt(const QString &extension);
extern bool isAvExt(const QString &extension);

class FileCacheHandler : public _ScheduledExecutor {
public:
    FileCacheHandler(const QString &filePath,
                     const bool &visibleInListWidgets = true);
    ~FileCacheHandler();

    const QString &getFilePath() {
        return mFilePath;
    }

    virtual void clearCache();

    void scheduleUpdateForAllDependent();
    void addDependentBox(BoundingBox *dependent);
    void removeDependentBox(BoundingBox *dependent);
    virtual void replace() {}
    void setVisibleInListWidgets(const bool &bT);
protected:
    bool mVisibleInListWidgets;
    QList<BoundingBoxQWPtr> mDependentBoxes;
    QString mFilePath;
    QString mUpdateFilePath;
};

class FileSourcesCache {
public:
    FileSourcesCache();

    static void addFileSourceListVisibleWidget(
            FileSourceListVisibleWidget *wid);
    static void removeFileSourceListVisibleWidget(
            FileSourceListVisibleWidget *wid);
    static FileCacheHandler *getHandlerForFilePath(const QString &filePath);
    static void removeHandler(FileCacheHandler *handler);
    static void clearAll();
    static int getFileCacheListCount();

    static void addHandlerToHandlersList(FileCacheHandler *handlerPtr);
    static void addHandlerToListWidgets(FileCacheHandler *handlerPtr);
    static void removeHandlerFromListWidgets(FileCacheHandler *handlerPtr);
private:
    static QList<FileSourceListVisibleWidget*> mFileSourceListVisibleWidgets;
    static QList<std::shared_ptr<FileCacheHandler> > mFileCacheHandlers;
};

class ImageCacheHandler : public FileCacheHandler {
public:
    ImageCacheHandler(const QString &filePath,
                      const bool &visibleSeparatly = true);

    void _processUpdate();
    void afterUpdate();
    void clearCache() {
        mImage.reset();
        FileCacheHandler::clearCache();
    }
    sk_sp<SkImage> getImage() { return mImage; }
private:
    sk_sp<SkImage> mUpdateImage;
    sk_sp<SkImage> mImage;
};

class AnimationCacheHandler : public FileCacheHandler {
public:
    AnimationCacheHandler(const QString &filePath) :
        FileCacheHandler(filePath) {}
    AnimationCacheHandler() :
        FileCacheHandler("") {}
    virtual sk_sp<SkImage> getFrameAtFrame(const int &relFrame) = 0;

    virtual _ScheduledExecutor *scheduleFrameLoad(const int &frame) = 0;
    const int &getFramesCount() { return mFramesCount; }
protected:
    int mFramesCount = 0;
    virtual void updateFrameCount() = 0;
};

class ImageSequenceCacheHandler : public AnimationCacheHandler {
public:
    ImageSequenceCacheHandler(const QStringList &framePaths);

    sk_sp<SkImage> getFrameAtFrame(const int &relFrame);

    void updateFrameCount();

    void _processUpdate() {}

    void clearCache();

    _ScheduledExecutor *scheduleFrameLoad(const int &frame);
protected:
    QStringList mFramePaths;
    QList<std::shared_ptr<ImageCacheHandler> > mFrameImageHandlers;
};

class VideoCacheHandler : public AnimationCacheHandler {
public:
    VideoCacheHandler(const QString &filePath);

    sk_sp<SkImage> getFrameAtFrame(const int &relFrame);

    void beforeUpdate();

    void _processUpdate();

    void afterUpdate();

    void clearCache();

    const qreal &getFps();    

    virtual _ScheduledExecutor *scheduleFrameLoad(const int &frame);
protected:
    QList<int> mFramesLoadScheduled;

    QList<int> mFramesBeingLoadedGUI;
    QList<int> mFramesBeingLoaded;
    QList<sk_sp<SkImage> > mLoadedFrames;

    qreal mFps = 24.;
    int mTimeBaseDen = 1;
    int mTimeBaseNum = 24;

    qreal mUpdateFps = 24.;
    int mUpdateTimeBaseDen = 1;
    int mUpdateTimeBaseNum = 24;

    void updateFrameCount();

    CacheHandler mFramesCache;
};
#endif // FILESOURCESCACHE_H
