#ifndef SOUNDCACHEHANDLER_H
#define SOUNDCACHEHANDLER_H
#include "CacheHandlers/hddcachablecachehandler.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "Decode/audiodecode.h"

class SoundCacheHandler : public HDDCachableCacheHandler<SoundCacheContainer> {
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
public:
    void rangeNeeded(const SampleRange& range) {
        blockConts(range, true);
        auto missings = getMissingRanges(range);
        if(missings.isEmpty()) return;
        for(const auto& missing : missings) {
            loadSampleRange(missing);
        }
        mergeAdjecent();
    }
protected:
    void loadSampleRange(const SampleRange& range) {
        float * data = nullptr;
        gDecodeSoundDataRange(mFilePath.toLatin1().data(), range, data);
        auto samples = SPtrCreate(Samples)(data, range.span());
        createNew
                <SoundCacheContainer>(range, samples);
    }

    void merge(const stdptrSCC& a, const stdptrSCC& b) {
        Q_ASSERT(a->getRange().neighbours(b->getRange()));
        stdptrSCC aS = a;
        stdptrSCC bS = b;
        removeRenderContainer(a);
        removeRenderContainer(b);
        const auto cont = SoundCacheContainer::sCreateMerge(aS, bS, this);
        auto frameRange = cont->getRange();
        int contId = insertIdForRelFrame(frameRange.fMin);
        mRenderContainers.insert(contId, cont);
    }

    void mergeAdjecent() {
        for(int i = 0; i < mRenderContainers.count() - 1; i++) {
            const auto& a = mRenderContainers.at(i);
            const auto& b = mRenderContainers.at(i + 1);

            if(a->getRange().neighbours(b->getRange())) {
                merge(GetAsSPtr(a, SoundCacheContainer),
                      GetAsSPtr(b, SoundCacheContainer)); i--;
            }
        }
    }

    const QString mFilePath;
};

#endif // SOUNDCACHEHANDLER_H
