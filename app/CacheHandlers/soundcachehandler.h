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
//    void clearCache();
//    void replace();

    void setFilePath(const QString& path) { // throw
        clearCache();
        FileCacheHandler::setFilePath(path);
        openAudioStream();
    }

    stdsptr<Samples> getSamplesForSecond(const int &secondId);

    void secondReaderFinished(const int& secondId,
                              const stdsptr<Samples>& samples);
    void secondReaderCanceled(const int &secondId) {
        removeSecondReader(secondId);
    }

    SoundReaderForMerger * getSecondReader(const int& second) {
        const int id = mSecondsBeingRead.indexOf(second);
        if(id >= 0) return mSecondReaders.at(id).get();
        return nullptr;
    }

    SoundReaderForMerger * addSecondReader(const int& secondId);

    int durationSec() const {
        if(!mAudioStreamsData) return 0;
        return mAudioStreamsData->fDurationSec;
    }

    const HDDCachableCacheHandler& getCacheHandler() const {
        return mSecondsCache;
    }
protected:
    void removeSecondReader(const int& second) {
        const int id = mSecondsBeingRead.indexOf(second);
        mSecondsBeingRead.removeAt(id);
        mSecondReaders.removeAt(id);
    }

private:
    void openAudioStream() {
        mAudioStreamsData = AudioStreamsData::sOpen(mFilePath);
    }

    QList<int> mSecondsBeingRead;
    QList<stdsptr<SoundReaderForMerger>> mSecondReaders;
    stdsptr<const AudioStreamsData> mAudioStreamsData;

    HDDCachableCacheHandler mSecondsCache;
};

#endif // SOUNDCACHEHANDLER_H
