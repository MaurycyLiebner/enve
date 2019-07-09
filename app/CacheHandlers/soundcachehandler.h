#ifndef SOUNDCACHEHANDLER_H
#define SOUNDCACHEHANDLER_H
#include "FileCacheHandlers/filecachehandler.h"
#include "CacheHandlers/hddcachablecachehandler.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "FileCacheHandlers/audiostreamsdata.h"
#include "FileCacheHandlers/soundreader.h"
class SoundHandler;
class SingleSound;

class SoundCacheHandler : public FileDataCacheHandler {
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
    friend class StdSelfRef;
public:
    void clearCache() {}
    void replace() {}
    void afterSourceChanged();

    const HDDCachableCacheHandler& getCacheHandler() const {
        return mSecondsCache;
    }

    void addSecondReader(const int secondId,
                         const stdsptr<SoundReaderForMerger>& reader) {
        mSecondsBeingRead << secondId;
        mSecondReaders << reader;
    }

    stdsptr<Samples> getSamplesForSecond(const int secondId) {
        const auto cont = mSecondsCache.atFrame
                <SoundCacheContainer>(secondId);
        if(!cont) return nullptr;
        return cont->getSamples();
    }

    SoundReaderForMerger * getSecondReader(const int second) {
        const int id = mSecondsBeingRead.indexOf(second);
        if(id >= 0) return mSecondReaders.at(id).get();
        return nullptr;
    }

    void removeSecondReader(const int second) {
        const int id = mSecondsBeingRead.indexOf(second);
        mSecondsBeingRead.removeAt(id);
        mSecondReaders.removeAt(id);
    }

    void secondReaderFinished(const int secondId,
                             const stdsptr<Samples>& samples) {
        mSecondsCache.add(SPtrCreate(SoundCacheContainer)(
                              samples, iValueRange{secondId, secondId},
                              &mSecondsCache));
    }
private:
    QList<SoundHandler*> mSoundHandlers;
    QList<int> mSecondsBeingRead;
    QList<stdsptr<SoundReaderForMerger>> mSecondReaders;
    HDDCachableCacheHandler mSecondsCache;
};

class SoundHandler : public StdSelfRef {
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
    friend class StdSelfRef;
public:
    SoundHandler(SoundCacheHandler* const dataHandler) :
        mDataHandler(dataHandler) {
        openAudioStream();
    }

    void afterSourceChanged() {
        openAudioStream();
    }

    stdsptr<Samples> getSamplesForSecond(const int secondId);

    void secondReaderFinished(const int secondId,
                              const stdsptr<Samples>& samples);
    void secondReaderCanceled(const int secondId) {
        removeSecondReader(secondId);
    }

    SoundReaderForMerger * getSecondReader(const int second) {
        return mDataHandler->getSecondReader(second);
    }

    SoundReaderForMerger * addSecondReader(const int secondId);

    int durationSec() const {
        if(!mAudioStreamsData) return 0;
        return mAudioStreamsData->fDurationSec;
    }

    SoundCacheHandler* getDataHandler() const {
        return mDataHandler;
    }
    const HDDCachableCacheHandler& getCacheHandler() const {
        return mDataHandler->getCacheHandler();
    }

    const QString& getFilePath() const {
        return mDataHandler->getFilePath();
    }
protected:
    void removeSecondReader(const int second) {
        mDataHandler->removeSecondReader(second);
    }
private:
    void openAudioStream() {
        const auto filePath = mDataHandler->getFilePath();
        mAudioStreamsData = AudioStreamsData::sOpen(filePath);
    }

    SoundCacheHandler* const mDataHandler;
    stdsptr<AudioStreamsData> mAudioStreamsData;

};

#endif // SOUNDCACHEHANDLER_H
