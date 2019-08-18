#include "soundcomposition.h"
#include "singlesound.h"
#include "castmacros.h"
#include "canvas.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "soundmerger.h"
#include "FileCacheHandlers/soundreader.h"

SoundComposition::SoundComposition(Canvas * const parent) :
    QIODevice(parent), mParent(parent) {}

void SoundComposition::start(const int startFrame) {
    mPos = qRound(startFrame/mParent->getFps()*SOUND_SAMPLERATE);
    open(QIODevice::ReadOnly);
}

void SoundComposition::stop() {
    close();
    unblockAll();
}

void SoundComposition::addSound(const qsptr<SingleSound>& sound) {
    connect(sound.get(), &Property::prp_absFrameRangeChanged,
            this, &SoundComposition::frameRangeChanged);
    mSounds.append(sound);
    frameRangeChanged(sound->prp_absInfluenceRange());
}

void SoundComposition::removeSound(const qsptr<SingleSound>& sound) {
    disconnect(sound.get(), &Property::prp_absFrameRangeChanged,
               this, &SoundComposition::frameRangeChanged);
    mSounds.removeOne(sound);
    frameRangeChanged(sound->prp_absInfluenceRange());
}

void SoundComposition::secondFinished(const int secondId,
                                      const stdsptr<Samples> &samples) {
    mProcessingSeconds.removeOne(secondId);
    if(!samples) return;
    const auto sCont = enve::make_shared<SoundCacheContainer>(
                samples, iValueRange{secondId, secondId}, &mSecondsCache);
    mSecondsCache.add(sCont);
    if(mBlockRange.inRange(secondId)) sCont->setBlocked(true);
}

void SoundComposition::startBlockingAtFrame(const int frame) {
    if(mBlockRange.isValid()) unblockAll();
    const qreal fps = mParent->getFps();
    const int sec = qFloor(frame/fps);
    mBlockRange = {sec, sec};
    mSecondsCache.blockConts(mBlockRange, true);
}

void SoundComposition::blockUpToFrame(const int frame) {
    const qreal fps = mParent->getFps();
    const int sec = qFloor(frame/fps);
    if(sec < mBlockRange.fMax) {
        mSecondsCache.blockConts({sec + 1, mBlockRange.fMax}, false);
    } else mSecondsCache.blockConts({mBlockRange.fMax + 1, sec}, true);
    mBlockRange.fMax = sec;
}

void SoundComposition::unblockAll() {
    if(mBlockRange.isValid())
        mSecondsCache.blockConts(mBlockRange, false);
    mBlockRange = {0, -1};
}

void SoundComposition::scheduleFrameRange(const FrameRange &range) {
    const qreal fps = mParent->getFps();
    const int minSec = qFloor((range.fMin + 1)/fps);
    const int maxSec = qFloor((range.fMax + 1)/fps);
    for(int i = minSec; i <= maxSec; i++) scheduleSecond(i);
}

SoundMerger *SoundComposition::scheduleFrame(const int frameId) {
    const qreal fps = mParent->getFps();
    return scheduleSecond(qFloor(frameId/fps));
}

SoundMerger *SoundComposition::scheduleSecond(const int secondId) {
    if(mSounds.isEmpty()) return nullptr;
    if(mProcessingSeconds.contains(secondId)) return nullptr;
    if(mSecondsCache.atFrame(secondId)) return nullptr;
    mProcessingSeconds.append(secondId);
    const SampleRange sampleRange = {secondId*SOUND_SAMPLERATE,
                                     (secondId + 1)*SOUND_SAMPLERATE - 1};
    const qreal fps = mParent->getFps();

    const auto task = enve::make_shared<SoundMerger>(secondId, sampleRange, this);
    for(const auto &sound : mSounds) {
        if(!sound->isEnabled() || !sound->isVisible()) continue;
        const auto enabledFrameRange = sound->prp_absInfluenceRange();
        const iValueRange enabledSecRange{qFloor(enabledFrameRange.fMin/fps),
                                          qFloor(enabledFrameRange.fMax/fps)};
        if(!enabledSecRange.inRange(secondId)) continue;
        const auto secs = sound->absSecondToRelSeconds(secondId);
        for(int i = secs.fMin; i <= secs.fMax; i++) {
            const auto samples = sound->getSamplesForSecond(i);
            if(samples) {
                task->addSoundToMerge({sound->getSampleShift(),
                                       sound->absSampleRange(),
                                       sound->getVolumeSnap(),
                                       sound->getStretch(),
                                       enve::make_shared<Samples>(samples)});
            } else {
                const auto reader = sound->getSecondReader(i);
                if(!reader) continue;
                reader->addMerger(task.get());
                reader->addDependent(task.get());
                reader->addSingleSound(sound->getSampleShift(),
                                       sound->absSampleRange(),
                                       sound->getVolumeSnap(),
                                       sound->getStretch());
            }
        }
    }
    task->scheduleTask();
    return task.get();
}

void SoundComposition::frameRangeChanged(const FrameRange &range) {
    const qreal fps = mParent->getFps();
    secondRangeChanged({qFloor(range.fMin/fps), qCeil(range.fMax/fps)});
}

qint64 SoundComposition::readData(char *data, qint64 maxLen) {
    qint64 total = 0;
    const SampleRange readSamples{static_cast<int>(mPos),
                                  static_cast<int>(mPos + maxLen/qint64(sizeof(float)))};
    while(maxLen > total) {
        const int secondId = static_cast<int>(mPos/SOUND_SAMPLERATE);
        const auto cont = mSecondsCache.atFrame<SoundCacheContainer>(secondId);
        if(!cont) break;
        const auto contSampleRange = cont->getSamples()->fSampleRange;
        const auto secondData = cont->getSamplesData();
        if(!secondData) break;
        const SampleRange samplesToRead = readSamples*contSampleRange;
        const SampleRange contRelRange =
                samplesToRead.shifted(-contSampleRange.fMin);
        const int nSamples = contRelRange.span();
        const qint64 chunk = qMin(maxLen - total, nSamples*qint64(sizeof(float)));
        memcpy(data + total, secondData + contRelRange.fMin, static_cast<size_t>(chunk));
        mPos += nSamples;
        total += chunk;
    }

    return total;
}

qint64 SoundComposition::writeData(const char *data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}
