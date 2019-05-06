#ifndef SOUNDCACHEHANDLER_H
#define SOUNDCACHEHANDLER_H
#include "FileCacheHandlers/filecachehandler.h"
#include "CacheHandlers/hddcachablecachehandler.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "Decode/audiodecode.h"
#include "FileCacheHandlers/audiostreamsdata.h"
#include "FileCacheHandlers/soundreader.h"
class SoundCacheHandler;
class SingleSound;

class SoundCacheHandler : public FileCacheHandler {
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
    friend class StdSelfRef;
public:
    void clearCache();
    void replace();

    void setFilePath(const QString& path) { // throw
        clearCache();
        FileCacheHandler::setFilePath(path);
        openAudioStream();
    }

    stdsptr<Samples> getSamplesForSecond(const int &secondId);
    Task *scheduleSecondLoad(const int &secondId);

    void secondLoaderFinished(const int& secondId,
                              const stdsptr<Samples>& samples);
    void secondLoaderCanceled(const int &secondId) {
        removeSecondLoader(secondId);
    }
protected:
    SoundReader * getSecondLoader(const int& second) {
        const int id = mSecondsBeingLoaded.indexOf(second);
        if(id >= 0) return mSecondLoaders.at(id).get();
        return nullptr;
    }

    SoundReader * addSecondLoader(const int& secondId);

    void removeSecondLoader(const int& second) {
        const int id = mSecondsBeingLoaded.indexOf(second);
        mSecondsBeingLoaded.removeAt(id);
        mSecondLoaders.removeAt(id);
    }

private:
    void openAudioStream() {
        mAudioStreamsData = AudioStreamsData::sOpen(mFilePath);
        mSecondCount = mAudioStreamsData->fDurationSec;
    }

    int mSecondCount;
    QString mFilePath;

    QList<int> mSecondsBeingLoaded;
    QList<stdsptr<SoundReader>> mSecondLoaders;
    stdsptr<const AudioStreamsData> mAudioStreamsData;
    const SingleSound * const mSingleSound;

    HDDCachableCacheHandler mSecondsCache;
};

#endif // SOUNDCACHEHANDLER_H
