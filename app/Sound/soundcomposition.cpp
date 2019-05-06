#include "soundcomposition.h"
#include "singlesound.h"
#include "castmacros.h"
#include "canvas.h"

SoundComposition::SoundComposition(Canvas * const parent) :
    QIODevice(parent), mParent(parent) {
    connect(mSoundsAnimatorContainer.get(),
            &Property::prp_absFrameRangeChanged,
            this, &SoundComposition::frameRangeChanged);
}

void SoundComposition::start() {
    open(QIODevice::ReadOnly);
}

void SoundComposition::stop() {
    mPos = 0;
    close();
    mBuffer.clear();
}

void SoundComposition::generateData(const int &startAbsFrame,
                                    const int &endAbsFrame,
                                    const qreal &fps) {
    if(mSounds.isEmpty()) return;

    uint nSamples = static_cast<uint>(
                qCeil((endAbsFrame - startAbsFrame)*SOUND_SAMPLERATE/fps));
    //float *data1 = nullptr;
    float *data = new float[nSamples];
    for(uint i = 0; i < nSamples; i++) {
        data[i] = 0;
    }

    for(const auto &sound : mSounds) {
        sound->updateFinalDataIfNeeded(fps, startAbsFrame, endAbsFrame);
        const int &soundStartFrame = sound->getStartAbsFrame();
        const int &soundSampleCount = sound->getSampleCount();
        int firstSampleFromSound;
        int sampleCountNeeded;
        int firstTargetSample;
        int samplesInSoundFrameRange =
                        qCeil((endAbsFrame - soundStartFrame)*SOUND_SAMPLERATE/fps);

        if(soundStartFrame >= startAbsFrame) {
            firstTargetSample =
                        qRound((soundStartFrame - startAbsFrame)*SOUND_SAMPLERATE/fps);
            firstSampleFromSound = 0;
            sampleCountNeeded = qMin(soundSampleCount,
                                     samplesInSoundFrameRange);
        } else {
            firstTargetSample = 0;
            firstSampleFromSound =
                        qRound((startAbsFrame - soundStartFrame)*SOUND_SAMPLERATE/fps);
            sampleCountNeeded = qMin(soundSampleCount - firstSampleFromSound,
                                     samplesInSoundFrameRange);
        }
        if(sampleCountNeeded <= 0) continue;
        const int lastSampleFromSound = firstSampleFromSound + sampleCountNeeded;
        int currTargetSample = firstTargetSample;
        const float *soundData = sound->getFinalData();
        for(int i = firstSampleFromSound; i < lastSampleFromSound; i++) {
            data[currTargetSample] = data[currTargetSample] + soundData[i];
            currTargetSample++;
        }
    }

    mBuffer.setRawData(rcChar(data), nSamples*sizeof(float));
    mPos = 0;
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

void SoundComposition::frameRangeChanged(const FrameRange &range) {
    const qreal fps = mParent->getFps();
    secondRangeChanged({qFloor(range.fMin/fps), qCeil(range.fMax/fps)});
}

ComplexAnimator *SoundComposition::getSoundsAnimatorContainer() {
    return mSoundsAnimatorContainer.get();
}

qint64 SoundComposition::readData(char *data, qint64 len) {
    qint64 total = 0;
    if(!mBuffer.isEmpty()) {
        while(len - total > 0) {
            const qint64 chunk = qMin((mBuffer.size() - mPos), len - total);
            memcpy(data + total, mBuffer.constData() + mPos,
                   static_cast<size_t>(chunk));
            mPos = (mPos + chunk) % mBuffer.size();
            total += chunk;
        }
    }
    return total;
}

qint64 SoundComposition::writeData(const char *data, qint64 len) {
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 SoundComposition::bytesAvailable() const {
    return mBuffer.size() + QIODevice::bytesAvailable();
}
