#ifndef IMAGESEQUENCECACHEHANDLER_H
#define IMAGESEQUENCECACHEHANDLER_H
#include "imagecachehandler.h"
#include "animationcachehandler.h"

class ImageSequenceFileHandler : public FileCacheHandler {
protected:
    void afterPathSet(const QString& folderPath);
    void reload();
public:
    void replace(QWidget * const parent);

    sk_sp<SkImage> getFrameAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame);
    eTask* scheduleFrameLoad(const int frame);
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
    eTask* scheduleFrameLoad(const int frame) {
        if(!mFileHandler) return nullptr;
        return mFileHandler->scheduleFrameLoad(frame);
    }
    void reload() {
        if(mFileHandler) mFileHandler->reloadAction();
    }
    int getFrameCount() const {
        if(!mFileHandler) return 0;
        return mFileHandler->getFrameCount();
    }
private:
    qptr<ImageSequenceFileHandler> mFileHandler;

};
#endif // IMAGESEQUENCECACHEHANDLER_H
