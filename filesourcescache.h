#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include <unordered_map>
#include <QString>
#include <QList>
#include "skiaincludes.h"
#include "Boxes/rendercachehandler.h"
#include "updatable.h"
typedef QSharedPointer<BoundingBox> BoundingBoxQSPtr;
class FileSourceListVisibleWidget;

extern bool isVideoExt(const QString &extension);
extern bool isSoundExt(const QString &extension);
extern bool isVectorExt(const QString &extension);
extern bool isImageExt(const QString &extension);
extern bool isAvExt(const QString &extension);

class FileCacheHandler : public Updatable {
public:
    FileCacheHandler(const QString &filePath);
    ~FileCacheHandler();

    const QString &getFilePath() {
        return mFilePath;
    }

    virtual void clearCache();

    void scheduleUpdateForAllDependent();
    void addDependentBox(BoundingBox *dependent);
    void removeDependentBox(BoundingBox *dependent);
    virtual void replace() {}
protected:
    QList<BoundingBoxQSPtr> mDependentBoxes;
    std::shared_ptr<FileCacheHandler> mFileHandlerRef;
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
    static void addHandler(FileCacheHandler *handlerPtr);
    static FileCacheHandler *getHandlerForFilePath(const QString &filePath);
    static void removeHandler(FileCacheHandler *handler);
    static void clearAll();
    static const QList<FileCacheHandler*> &getFileCacheList();

private:
    static QList<FileSourceListVisibleWidget*> mFileSourceListVisibleWidgets;
    static QList<FileCacheHandler*> mFileCacheHandlers;
};

class ImageCacheHandler : public FileCacheHandler {
public:
    ImageCacheHandler(const QString &filePath);

    void processUpdate();
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

    virtual Updatable *scheduleFrameLoad(const int &frame) = 0;
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

    void processUpdate() {}

    void clearCache();

    Updatable *scheduleFrameLoad(const int &frame);
protected:
    QStringList mFramePaths;
    QList<ImageCacheHandler*> mFrameImageHandlers;
};

class VideoCacheHandler : public AnimationCacheHandler {
public:
    VideoCacheHandler(const QString &filePath);

    sk_sp<SkImage> getFrameAtFrame(const int &relFrame);

    void beforeUpdate();

    void processUpdate();

    void afterUpdate();

    void clearCache();

    const qreal &getFps();    

    virtual Updatable *scheduleFrameLoad(const int &frame);
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
