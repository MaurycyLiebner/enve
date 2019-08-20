#include "soundcachehandler.h"
#include "FileCacheHandlers/soundreader.h"
#include "Sound/singlesound.h"

SoundDataHandler::SoundDataHandler() {
    connect(&eSoundSettings::sSettings, &eSoundSettings::settingsChanged,
            this, [this]() {
        mSecondsCache.clear();
    });
}

stdsptr<Samples> SoundHandler::getSamplesForSecond(const int secondId) {
    return mDataHandler->getSamplesForSecond(secondId);
}

void SoundHandler::secondReaderFinished(
        const int secondId,
        const stdsptr<Samples>& samples) {
    if(samples) mDataHandler->secondReaderFinished(secondId, samples);
    removeSecondReader(secondId);
}

SoundReaderForMerger *SoundHandler::addSecondReader(const int secondId) {
    const int sampleRate = eSoundSettings::sSampleRate();
    const SampleRange range = {secondId*sampleRate, (secondId + 1)*sampleRate - 1};
    const auto reader = enve::make_shared<SoundReaderForMerger>(
                this, mAudioStreamsData, secondId, range);
    mDataHandler->addSecondReader(secondId, reader);
    reader->scheduleTask();
    return reader.get();
}

void SoundDataHandler::afterSourceChanged() {
    for(const auto& handler : mSoundHandlers) {
        handler->afterSourceChanged();
    }
}

#include <QFileDialog>
void SoundFileHandler::replace(QWidget * const parent) {
    const auto importPath = QFileDialog::getOpenFileName(
                parent, "Change Source", mPath,
                "Audio Files (*.wav *.mp3)");
    if(!importPath.isEmpty()) setPath(importPath);
}
