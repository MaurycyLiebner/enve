#include "soundcomposition.h"
#include "singlesound.h"
#include "castmacros.h"
#include "canvas.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "soundmerger.h"

SoundComposition::SoundComposition(Canvas * const parent) :
    QIODevice(parent), mParent(parent) {
    connect(mSoundsAnimatorContainer.get(),
            &Property::prp_absFrameRangeChanged,
            this, &SoundComposition::frameRangeChanged);
}

void SoundComposition::start(const int& startFrame) {
    mPos = qRound(startFrame/mParent->getFps()*SOUND_SAMPLERATE);
    open(QIODevice::ReadOnly);
}

void SoundComposition::stop() {
    close();
}

void SoundComposition::addSound(const qsptr<SingleSound>& sound) {
    mSounds.append(sound);
}

void SoundComposition::removeSound(const qsptr<SingleSound>& sound) {
    mSounds.removeOne(sound);
}

void SoundComposition::addSoundAnimator(const qsptr<SingleSound>& sound) {
    addSound(sound);
    mSoundsAnimatorContainer->ca_addChildAnimator(sound);
}

void SoundComposition::removeSoundAnimator(const qsptr<SingleSound>& sound) {
    if(mSounds.removeOne(sound)) {
        mSoundsAnimatorContainer->ca_removeChildAnimator(sound);
    }
}

void SoundComposition::secondFinished(const int &secondId,
                                      const stdsptr<Samples> &samples) {
    qDebug() << "sec finished" << secondId << (samples != nullptr);
    mProcessingSeconds.removeOne(secondId);
    if(!samples) return;
    mSecondsCache.createNew<SoundCacheContainer>(secondId, samples)->setBlocked(true);
}

SoundMerger *SoundComposition::scheduleFrame(const int &frameId) {
    const qreal fps = mParent->getFps();
    return scheduleSecond(qFloor(frameId/fps));
}

SoundMerger *SoundComposition::scheduleSecond(const int &secondId) {
    if(mSounds.isEmpty()) return nullptr;
    if(mProcessingSeconds.contains(secondId)) return nullptr;
    if(mSecondsCache.atRelFrame(secondId)) return nullptr;
    mProcessingSeconds.append(secondId);
    const SampleRange sampleRange = {secondId*SOUND_SAMPLERATE,
                                     (secondId + 1)*SOUND_SAMPLERATE - 1};
    const auto task = SPtrCreate(SoundMerger)(secondId, sampleRange, this);
    for(const auto &sound : mSounds) {
        const auto secs = sound->absSecondToRelSeconds(secondId);
        for(int i = secs.fMin; i <= secs.fMax; i++) {
            auto reader = sound->getSecondReader(i);
            if(!reader) reader = sound->addSecondReader(i, task.get());
            reader->addSingleSound(sound->getSampleShift(),
                                   sound->absSampleRange());
        }
    }
    task->scheduleTask();
    qDebug() << "sec scheduled" << secondId;
    return task.get();
}

void SoundComposition::frameRangeChanged(const FrameRange &range) {
    const qreal fps = mParent->getFps();
    secondRangeChanged({qFloor(range.fMin/fps), qCeil(range.fMax/fps)});
}

ComplexAnimator *SoundComposition::getSoundsAnimatorContainer() {
    return mSoundsAnimatorContainer.get();
}

qint64 SoundComposition::readData(char *data, qint64 maxLen) {
    qint64 total = 0;
    const SampleRange readSamples{static_cast<int>(mPos),
                                  static_cast<int>(mPos + maxLen/sizeof(float))};
    while(maxLen > total) {
        const int secondId = mPos/SOUND_SAMPLERATE;
        const auto cont = mSecondsCache.atRelFrame<SoundCacheContainer>(secondId);
        if(!cont) break;
        const auto contSampleRange = cont->getSamples()->fSampleRange;
        const auto secondData = cont->getSamplesData();
        if(!secondData) break;
        const SampleRange samplesToRead = readSamples*contSampleRange;
        const SampleRange contRelRange =
                samplesToRead.shifted(-contSampleRange.fMin);
        const int nSamples = contRelRange.span();
        const qint64 chunk = qMin(maxLen - total,
                                  static_cast<qint64>(nSamples*sizeof(float)));
        memcpy(data + total, secondData + contRelRange.fMin,
               static_cast<size_t>(chunk));
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
