#ifndef FILESOURCESCACHE_H
#define FILESOURCESCACHE_H
#include "skimagecopy.h"
#include <unordered_map>
#include <QString>
#include <QList>
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

    bool isFileMissing() {
        return mFileMissing;
    }
protected:
    bool mFileMissing = false;
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
    sk_sp<SkImage> getImageCopy() {
        return makeSkImageCopy(mImage);
    }
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
    virtual sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame) = 0;
    sk_sp<SkImage> getFrameCopyAtFrame(const int &relFrame) {
        sk_sp<SkImage> imageToCopy = getFrameAtFrame(relFrame);
        return makeSkImageCopy(imageToCopy);
    }

    sk_sp<SkImage> getFrameCopyAtOrBeforeFrame(const int &relFrame) {
        sk_sp<SkImage> imageToCopy = getFrameAtOrBeforeFrame(relFrame);
        return makeSkImageCopy(imageToCopy);
    }

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
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame);

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
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame);

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


class SoundCacheHandler : public FileCacheHandler {
public:
    struct SoundDataRange {
        SoundDataRange(const int &minRelFrameT,
                       const int &maxRelFrameT) {
            minRelFrame = minRelFrameT;
            maxRelFrame = maxRelFrameT;
        }

        SoundDataRange(const int64_t &minSampleT,
                       const int64_t &maxSampleT,
                       const int &minRelFrameT,
                       const int &maxRelFrameT,
                       float *dataT) {
            minSample = minSampleT;
            maxSample = maxSampleT;
            updateSampleCount();
            minRelFrame = minRelFrameT;
            maxRelFrame = maxRelFrameT;
            data = dataT;
        }

        bool clipOverlapFrames(int *minFrame, int *maxFrame) const {
            int minFrameVal = *minFrame;
            int maxFrameVal = *maxFrame;
            if(minFrameVal >= minRelFrame && maxFrameVal <= maxRelFrame) {
                *minFrame = -1;
                *maxFrame = -1;
            } else if(minFrameVal >= minRelFrame && minFrameVal < maxRelFrame) {
                *minFrame = maxRelFrame;
            } else if(maxFrameVal > minRelFrame && maxFrameVal <= maxRelFrame) {
                *maxFrame = minRelFrame;
            } else {
                return false;
            }
            return true;
        }

        bool relFrameInRange(const int &relFrame) const {
            return relFrame >= minRelFrame && relFrame <= maxRelFrame;
        }

        int64_t minSample = 0;
        int64_t maxSample = 0;
        int64_t sampleCount = 0;

        int minRelFrame = 0;
        int maxRelFrame = 0;
        qreal fps;
        float *data = nullptr;

        bool canBeMergedWith(const SoundDataRange &range) {
            return range.minRelFrame == maxRelFrame ||
                    range.maxRelFrame == minRelFrame;
        }

        bool overlapsWith(const SoundDataRange &range) { // assumes neither is contained in the other
            if(minSample <= range.minSample && range.minSample <= maxSample) {
                return true;
            } else if(minSample <= range.maxSample && range.maxSample <= maxSample) {
                return true;
            }
            return false;
        }

        void updateSampleCount() {
            sampleCount = maxSample - minSample + 1;
        }

        void removeOverlapWith(const SoundDataRange &range) { // assumes neither is contained in the other
            if(minSample <= range.minSample && range.minSample <= maxSample) {
                maxSample = range.minSample - 1;
            } else if(minSample <= range.maxSample && range.maxSample <= maxSample) {
                 minSample = range.maxSample + 1;
            } else {
                return;
            }
            updateSampleCount();
        }

        bool mergeWithRange(const SoundDataRange &range) {
            if(!canBeMergedWith(range)) return false;
            removeOverlapWith(range);
            float *newData = nullptr;
            newData = new float[range.sampleCount + sampleCount];
            if(range.minSample > minSample) {
                int j = 0;
                for(int i = 0; i < sampleCount; i++, j++) {
                    newData[j] = data[i];
                }
                for(int i = 0; i < range.sampleCount; i++, j++) {
                    newData[j] = range.data[i];
                }
                maxRelFrame = range.maxRelFrame;
                maxSample = range.maxSample;
            } else {
                int j = 0;
                for(int i = 0; i < range.sampleCount; i++, j++) {
                    newData[j] = range.data[i];
                }
                for(int i = 0; i < sampleCount; i++, j++) {
                    newData[j] = data[i];
                }
                minRelFrame = range.minRelFrame;
                minSample = range.minSample;
            }
            sampleCount += range.sampleCount;
            delete[] data;
            data = newData;
            return true;
        }
    };
    SoundCacheHandler(const QString &filePath);

    void beforeUpdate() {
        FileCacheHandler::beforeUpdate();
        //qDebug() << "loading: " << mFramesLoadScheduled;
        mSoundBeingLoaded = mScheduledSoundLoad;
        mSoundBeingLoadedGUI = mSoundBeingLoaded;
        mScheduledSoundLoad.clear();
        mUpdateFilePath = mFilePath;
        mUpdateFps = mFps;
        mUpdateTimeBaseDen = mTimeBaseDen;
        mUpdateTimeBaseNum = mTimeBaseNum;
    }

    void _processUpdate();

    void afterUpdate() {
        foreach(SoundDataRange rangeT, mSoundBeingLoadedGUI) {
            int targetIdT;
            getInsertIdForRelFrame(rangeT.minRelFrame, &targetIdT, mSoundData);
            int maxI = targetIdT + 1;
            int minI = targetIdT - 1;
            for(int i = minI; i <= maxI; i++) {
                if(i < 0) continue;
                if(i >= mSoundData.count()) break;
                const SoundDataRange &range = mSoundData.at(i);
                if(rangeT.mergeWithRange(range)) {
                    mSoundData.removeAt(i);
                    if(i < targetIdT) targetIdT--;
                    i--;
                    maxI--;
                }
            }
            mSoundData.insert(targetIdT, rangeT);
        }
        mSoundBeingLoadedGUI.clear();
        mSoundBeingLoaded.clear();
    }

    void clearCache() {}

    const qreal &getFps();

    _ScheduledExecutor *scheduleSoundRangeLoad(int minRelFrame,
                                int maxRelFrame) {
        SoundDataRange newDataRangeToLoad = SoundDataRange(minRelFrame,
                                                           maxRelFrame);
        int targetIdT;
        getInsertIdForRelFrame(minRelFrame, &targetIdT,
                                       mSoundData);
        int maxI = targetIdT + 1;
        int minI = targetIdT - 1;
        for(int i = minI; i <= maxI; i++) {
            if(i < 0) continue;
            if(i >= mSoundData.count()) break;
            const SoundDataRange &range = mSoundData.at(i);
            if(range.clipOverlapFrames(&minRelFrame, &maxRelFrame)) {
                if(minRelFrame >= maxRelFrame) return nullptr;
            }
        }

        getInsertIdForRelFrame(minRelFrame, &targetIdT,
                               mSoundBeingLoadedGUI);
        maxI = targetIdT + 1;
        minI = targetIdT - 1;
        for(int i = minI; i <= maxI; i++) {
            if(i < 0) continue;
            if(i >= mSoundBeingLoadedGUI.count()) break;
            const SoundDataRange &range = mSoundBeingLoadedGUI.at(i);
            if(range.clipOverlapFrames(&minRelFrame, &maxRelFrame)) {
                if(minRelFrame >= maxRelFrame) return nullptr;
            }
        }

        getInsertIdForRelFrame(minRelFrame, &targetIdT,
                               mScheduledSoundLoad);
        maxI = targetIdT + 1;
        minI = targetIdT - 1;
        for(int i = minI; i <= maxI; i++) {
            if(i < 0) continue;
            if(i >= mScheduledSoundLoad.count()) break;
            const SoundDataRange &range = mScheduledSoundLoad.at(i);
            if(range.clipOverlapFrames(&minRelFrame, &maxRelFrame)) {
                if(minRelFrame >= maxRelFrame) return nullptr;
                newDataRangeToLoad.minRelFrame = qMin(minRelFrame,
                                                      range.minRelFrame);
                newDataRangeToLoad.maxRelFrame = qMax(maxRelFrame,
                                                      range.maxRelFrame);
                mScheduledSoundLoad.removeAt(i);
                if(i < targetIdT) targetIdT--;
                i--;
                maxI--;
            }
        }
        mScheduledSoundLoad.insert(targetIdT, newDataRangeToLoad);
        return this;
    }

    void getInsertIdForRelFrame(const int &relFrame,
                                int *id,
                                QList<SoundDataRange> &dataList) {
        if(dataList.isEmpty()) {
            *id = 0;
            return;
        }
        int minId = 0;
        int maxId = dataList.count() - 1;

        while(minId <= maxId) {
            int guess = (minId + maxId)/2;
            const SoundDataRange &range = dataList.at(guess);
            if(range.relFrameInRange(relFrame)) {
                *id = guess;
                return;
            }
            int rangeMinFrame = range.minRelFrame;
            if(rangeMinFrame > relFrame) {
                if(maxId == guess) {
                    *id = minId;
                    return;
                } else {
                    maxId = guess;
                }
            } else if(rangeMinFrame < relFrame) {
                if(minId == guess) {
                    *id = maxId;
                    return;
                } else {
                    minId = guess;
                }
            } else {
                *id = guess;
                return;
            }
        }
    }
protected:
    int decodeSoundDataRange(SoundDataRange &range);

    QList<SoundDataRange> mScheduledSoundLoad;
    QList<SoundDataRange> mSoundBeingLoaded;
    QList<SoundDataRange> mSoundBeingLoadedGUI;
    QList<SoundDataRange> mSoundData;

    qreal mFps = 24.;
    int mTimeBaseDen = 1;
    int mTimeBaseNum = 24;

    qreal mUpdateFps = 24.;
    int mUpdateTimeBaseDen = 1;
    int mUpdateTimeBaseNum = 24;
};
#endif // FILESOURCESCACHE_H
