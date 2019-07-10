#ifndef IMAGESEQUENCECACHEHANDLER_H
#define IMAGESEQUENCECACHEHANDLER_H
#include "imagecachehandler.h"
#include "animationcachehandler.h"

class ImageSequenceFileHandler : public FileCacheHandler {
protected:
public:
    void afterPathSet(const QString& folderPath);

    sk_sp<SkImage> getFrameAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame);
    Task* scheduleFrameLoad(const int frame);
    void reload();
    void replace();
    int getFrameCount() const { return mFrameImageHandlers.count(); }
private:
    QString mFolderPath;
    QList<qsptr<ImageDataHandler>> mFrameImageHandlers;
};

class ImageSequenceCacheHandler : public AnimationFrameHandler {
    friend class SelfRef;
protected:
    ImageSequenceCacheHandler();
public:
    void setFolderPath(const QString& folderPath);

    sk_sp<SkImage> getFrameAtFrame(const int relFrame) {
        if(!mFileHandler) return nullptr;
        return mFileHandler->getFrameAtFrame(relFrame);
    }
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame) {
        if(!mFileHandler) return nullptr;
        return mFileHandler->getFrameAtOrBeforeFrame(relFrame);
    }
    Task* scheduleFrameLoad(const int frame) {
        if(!mFileHandler) return nullptr;
        return mFileHandler->scheduleFrameLoad(frame);
    }
    void reload() {
        if(mFileHandler) mFileHandler->reload();
    }
    int getFrameCount() const {
        if(!mFileHandler) return 0;
        return mFileHandler->getFrameCount();
    }
private:
    qptr<ImageSequenceFileHandler> mFileHandler;
    QString mFolderPath;

};
#endif // IMAGESEQUENCECACHEHANDLER_H
