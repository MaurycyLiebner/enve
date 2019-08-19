#include "audiohandler.h"
#include "Sound/soundcomposition.h"
AudioHandler* AudioHandler::sInstance = nullptr;

AudioHandler::AudioHandler() {
    Q_ASSERT(!sInstance);
    sInstance = this;
}

const int BufferSize = 32768;

QAudioFormat::SampleType toQtAudioFormat(const AVSampleFormat avFormat) {
    if(avFormat == AV_SAMPLE_FMT_S32) {
        return QAudioFormat::SignedInt;
    } else if(avFormat == AV_SAMPLE_FMT_FLT) {
        return QAudioFormat::Float;
    } else RuntimeThrow("Unsupported sample format " +
                        av_get_sample_fmt_name(avFormat));
}

void AudioHandler::initializeAudio() {
    if(mAudioOutput) delete mAudioOutput;

    mAudioBuffer = QByteArray(BufferSize, 0);

    mAudioDevice = QAudioDeviceInfo::defaultOutputDevice();
    mAudioFormat.setSampleRate(eSoundSettings::sSampleRate());
    mAudioFormat.setChannelCount(eSoundSettings::sChannelCount());
    mAudioFormat.setSampleSize(eSoundSettings::sBytesPerSample());
    mAudioFormat.setCodec("audio/pcm");
    mAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
    mAudioFormat.setSampleType(toQtAudioFormat(eSoundSettings::sSampleFormat()));

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
