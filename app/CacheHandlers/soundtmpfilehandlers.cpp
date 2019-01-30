#include "soundtmpfilehandlers.h"
#include "soundcachecontainer.h"

SoundContainerTmpFileDataLoader::SoundContainerTmpFileDataLoader(
        const qsptr<QTemporaryFile> &file,
        SoundCacheContainer *target) : mTargetCont(target) {
    mTmpFile = file;
}

void SoundContainerTmpFileDataLoader::_processUpdate() {
    if(mTmpFile->open()) {
        int size;
        mTmpFile->read(reinterpret_cast<char*>(&size), sizeof(int));
        mSamples = SPtrCreate(Samples)(size);
        mTmpFile->read(reinterpret_cast<char*>(mSamples->fData),
                       size*sizeof(float));

        mTmpFile->close();
    }
}

void SoundContainerTmpFileDataLoader::afterProcessingFinished() {
    mTargetCont->setDataLoadedFromTmpFile(mSamples);
    _ScheduledTask::afterProcessingFinished();
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
        mTmpFile->write(reinterpret_cast<const char*>(&mSamples->fSamplesCount),
                        sizeof(int));
        mTmpFile->write(reinterpret_cast<char*>(mSamples->fData),
                        mSamples->fSamplesCount*sizeof(float));

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
    _ScheduledTask::afterProcessingFinished();
}
