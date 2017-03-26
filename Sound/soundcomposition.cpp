#include "soundcomposition.h"

SoundComposition::SoundComposition(QObject *parent)
    :   QIODevice(parent) {
    mSoundsAnimatorContainer.incNumberPointers();
    mSoundsAnimatorContainer.setName("sounds");
}

SoundComposition::~SoundComposition() {
    foreach(SingleSound *sound, mSounds) {
        sound->decNumberPointers();
    }
}

void SoundComposition::start()
{
    open(QIODevice::ReadOnly);
}

void SoundComposition::stop() {
    mPos = 0;
    close();
    mBuffer.clear();
}

void SoundComposition::generateData(const int &startFrame,
                                    const int &endFrame,
                                    const qreal &fps) {
    if(mSounds.isEmpty()) return;

    int nSamples = (endFrame - startFrame)*SAMPLERATE/fps;
    //float *data1 = NULL;
    float *data = new float[nSamples];
    for(int i = 0; i < nSamples; i++) {
        data[i] = 0.f;
    }
//    int size;
//    decode_audio_file("/home/ailuropoda/lektor.wav",
//                      SAMPLERATE,
//                      &data1,
//                      &size);
//    float *data = new float[nSamples];
//    for(int i = 0; i < nSamples; i++) {
//        data[i] = data1[i];
//    }
//    free(data1);

    foreach(SingleSound *sound, mSounds) {
        sound->updateFinalDataIfNeeded(fps, startFrame, endFrame);
        const int &soundStartFrame = sound->getStartFrame();
        const int &soundSampleCount = sound->getSampleCount();
        int firstSampleFromSound;
        int sampleCountNeeded;
        int firstTargetSample;
        int samplesInSoundFrameRange = (endFrame - soundStartFrame)*SAMPLERATE/fps;

        if(soundStartFrame >= startFrame) {
            firstTargetSample = (soundStartFrame - startFrame)*SAMPLERATE/fps;
            firstSampleFromSound = 0;
            sampleCountNeeded = qMin(soundSampleCount,
                                     samplesInSoundFrameRange);
        } else {
            firstTargetSample = 0;
            firstSampleFromSound = (startFrame - soundStartFrame)*SAMPLERATE/fps;
            sampleCountNeeded = qMin(soundSampleCount - firstSampleFromSound,
                                     samplesInSoundFrameRange);
        }
        if(sampleCountNeeded <= 0) continue;
        int lastSampleFromSound = firstSampleFromSound + sampleCountNeeded;
        int currTargetSample = firstTargetSample;
        const float *soundData = sound->getFinalData();
        for(int i = firstSampleFromSound; i < lastSampleFromSound; i++) {
            data[currTargetSample] = data[currTargetSample] + soundData[i];
            currTargetSample++;
        }
    }

    mBuffer.setRawData((char*)data, nSamples*sizeof(float));
    mPos = 0;
}

void SoundComposition::addSound(SingleSound *sound) {
    mSounds.append(sound);
    sound->incNumberPointers();
}

void SoundComposition::removeSound(SingleSound *sound) {
    if(mSounds.removeOne(sound)) {
        sound->decNumberPointers();
    }
}

void SoundComposition::addSoundAnimator(SingleSound *sound) {
    addSound(sound);
    mSoundsAnimatorContainer.addChildAnimator(sound);
}

void SoundComposition::removeSoundAnimator(SingleSound *sound) {
    if(mSounds.removeOne(sound)) {
        sound->decNumberPointers();
        mSoundsAnimatorContainer.removeChildAnimator(sound);
    }
}

ComplexAnimator *SoundComposition::getSoundsAnimatorContainer() {
    return &mSoundsAnimatorContainer;
}

qint64 SoundComposition::readData(char *data, qint64 len) {
    qint64 total = 0;
    if(!mBuffer.isEmpty()) {
        while(len - total > 0) {
            const qint64 chunk = qMin((mBuffer.size() - mPos), len - total);
            memcpy(data + total, mBuffer.constData() + mPos, chunk);
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
