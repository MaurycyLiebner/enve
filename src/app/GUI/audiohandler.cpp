#include "audiohandler.h"
#include "Sound/soundcomposition.h"
AudioHandler* AudioHandler::sInstance = nullptr;

AudioHandler::AudioHandler() {
    Q_ASSERT(!sInstance);
    sInstance = this;
}

const int BufferSize = 32768;

void AudioHandler::initializeAudio() {
    mAudioBuffer = QByteArray(BufferSize, 0);

    mAudioDevice = QAudioDeviceInfo::defaultOutputDevice();
    mAudioFormat.setSampleRate(SOUND_SAMPLERATE);
    mAudioFormat.setChannelCount(1);
    mAudioFormat.setSampleSize(32);
    mAudioFormat.setCodec("audio/pcm");
    mAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
    mAudioFormat.setSampleType(QAudioFormat::Float);

    QAudioDeviceInfo info(mAudioDevice);
    if(!info.isFormatSupported(mAudioFormat)) {
        //RuntimeThrow("Default format not supported - trying to use nearest");
        mAudioFormat = info.nearestFormat(mAudioFormat);
    }

    mAudioOutput = new QAudioOutput(mAudioDevice, mAudioFormat, this);
}

void AudioHandler::startAudio() {
    mAudioIOOutput = mAudioOutput->start();
}

void AudioHandler::stopAudio() {
    //mAudioOutput->suspend();
    //mCurrentSoundComposition->stop();
    mAudioIOOutput = nullptr;
    mAudioOutput->stop();
    mAudioOutput->reset();
}

void AudioHandler::setVolume(const int value) {
    if(mAudioOutput) mAudioOutput->setVolume(qreal(value)/100);
}
