#include "soundcachehandler.h"
#include "FileCacheHandlers/soundreader.h"
#include "Sound/singlesound.h"

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
    const SampleRange& range = {secondId*SOUND_SAMPLERATE,
                                (secondId + 1)*SOUND_SAMPLERATE - 1};
    const auto reader = SPtrCreate(SoundReaderForMerger)(
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
