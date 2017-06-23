#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include <unordered_map>
#include <QString>
#include <QList>
#include "skiaincludes.h"
#include "Boxes/rendercachehandler.h"
#include "updatable.h"

class FileCacheHandler : public Updatable {
public:
    FileCacheHandler(const QString &filePath);
    ~FileCacheHandler();

    const QString &getFilePath() {
        return mFilePath;
    }

    void beforeUpdate();

    void afterUpdate();

    virtual void clearCache() = 0;
    void addScheduler();
    void schedulerProccessed();
protected:
    QString mFilePath;

    bool mLoadingScheduled = false;
    bool mLoadingData = false;
};

class FileSourcesCache {
public:
    FileSourcesCache();

    static void addHandler(FileCacheHandler *handlerPtr);
    static FileCacheHandler *getHandlerForFilePath(const QString &filePath);
    static void removeHandler(FileCacheHandler *handler);
private:
    static QList<FileCacheHandler*> mFileCacheHandlers;
};

class ImageCacheHandler : public FileCacheHandler {
public:
    ImageCacheHandler(const QString &filePath);

    void processUpdate();
    void afterUpdate();
    void clearCache() {
        mImage.reset();
    }
    sk_sp<SkImage> getImage() { return mImage; }
private:
    sk_sp<SkImage> mUpdateImage;
    sk_sp<SkImage> mImage;
};

class AnimationCacheHandler {
public:
    AnimationCacheHandler() {}
    virtual sk_sp<SkImage> getFrameAtFrame(const int &relFrame) = 0;

    virtual void clearCache() {}

    virtual void scheduleFrameLoad(const int &frame) = 0;
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

    void scheduleFrameLoad(const int &frame);
protected:
    QStringList mFramePaths;
    QList<ImageCacheHandler*> mFrameImageHandlers;
};

class VideoCacheHandler : public AnimationCacheHandler,
                            public FileCacheHandler {
public:
    VideoCacheHandler(const QString &filePath);

    sk_sp<SkImage> getFrameAtFrame(const int &relFrame);

    void beforeUpdate();

    void processUpdate();

    void afterUpdate();

    void clearCache();

    const qreal &getFps();

    virtual void scheduleFrameLoad(const int &frame);
protected:
    QList<int> mFramesLoadScheduled;

    QList<int> mFramesBeingLoadedGUI;
    QList<int> mFramesBeingLoaded;
    QList<sk_sp<SkImage> > mLoadedFrames;
    qreal mFps = 24.;

    void updateFrameCount();

    CacheHandler mFramesCache;
};
#endif // FILESOURCESCACHE_H
