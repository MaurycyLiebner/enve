#include "soundtmpfilehandlers.h"
#include "soundcachecontainer.h"
#include "castmacros.h"

SoundContainerTmpFileDataLoader::SoundContainerTmpFileDataLoader(
        const qsptr<QTemporaryFile> &file,
        SoundCacheContainer *target) : mTargetCont(target) {
    mTmpFile = file;
}

void SoundContainerTmpFileDataLoader::_processUpdate() {
    if(mTmpFile->open()) {
        int size;
        mTmpFile->read(rcChar(&size), sizeof(int));
        mSamples = SPtrCreate(Samples)(size);
        mTmpFile->read(rcChar(mSamples->fData),
                       size*static_cast<qint64>(sizeof(float)));

        mTmpFile->close();
    }
}

void SoundContainerTmpFileDataLoader::afterProcessingFinished() {
    mTargetCont->setDataLoadedFromTmpFile(mSamples);
}

SoundContainerTmpFileDataSaver::SoundContainerTmpFileDataSaver(
        const stdsptr<Samples> &samples,
        SoundCacheContainer *target) : mTargetCont(target) {
    mSamples = samples;
}

void SoundContainerTmpFileDataSaver::_processUpdate() {
    // mSavingFailed = true; return; // NO TMP FILES !!!
    mTmpFile = qsptr<QTemporaryFile>(new QTemporaryFile());
    if(mTmpFile->open()) {
        mTmpFile->write(rcConstChar(&mSamples->fSamplesCount),
                        sizeof(int));
        const qint64 writeBytes = mSamples->fSamplesCount*
                static_cast<qint64>(sizeof(float));
        mTmpFile->write(rcChar(mSamples->fData), writeBytes);

        mTmpFile->close();
    } else {
        mSavingFailed = true;
    }
}

void SoundContainerTmpFileDataSaver::afterProcessingFinished() {
    if(mSavingFailed) {
        if(!mTargetCont->freeAndRemove()) {

        }
    } else {
        mTargetCont->setDataSavedToTmpFile(mTmpFile);
    }
}
