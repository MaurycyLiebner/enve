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
protected:
    QString mFilePath;

    bool mLoadingScheduled = false;
    bool mLoadingData = false;

    void addScheduler();
};

class ImageCacheHandler : public FileCacheHandler {
public:
    ImageCacheHandler(const QString &filePath);

    void processUpdate();
    void afterUpdate();
private:
    sk_sp<SkImage> mUpdateImage;
    sk_sp<SkImage> mImage;
};

class VideoCacheHandler : public FileCacheHandler {
public:
    VideoCacheHandler(const QString &filePath);

    CacheContainer *getFrameCacheAtFrame(const int &relFrame);
    void scheduleFrameLoad(const int &frame);

    void beforeUpdate();

    void processUpdate();

    void afterUpdate();
private:

    qreal mFps = 24.;
    int mFramesCount = 0;

    void updateFrameCount();

    QList<int> mFramesBeingLoaded;
    QList<sk_sp<SkImage> > mLoadedFrames;

    QList<int> mFramesLoadScheduled;
    CacheHandler mFramesCache;
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

#endif // FILESOURCESCACHE_H
