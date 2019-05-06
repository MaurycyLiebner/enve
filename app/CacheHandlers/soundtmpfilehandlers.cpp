#include "soundtmpfilehandlers.h"
#include "soundcachecontainer.h"
#include "castmacros.h"

SoundContainerTmpFileDataLoader::SoundContainerTmpFileDataLoader(
        const qsptr<QTemporaryFile> &file,
        SoundCacheContainer *target) : mTargetCont(target) {
    mTmpFile = file;
}

void SoundContainerTmpFileDataLoader::processTask() {
    if(mTmpFile->open()) {
        SampleRange sampleRange;
        mTmpFile->read(rcChar(&sampleRange), sizeof(SampleRange));
        const size_t size = static_cast<size_t>(sampleRange.span());
        float * data = new float[size];
        mTmpFile->read(rcChar(data),
                       static_cast<qint64>(size*sizeof(float)));
        mSamples = SPtrCreate(Samples)(data, sampleRange);

        mTmpFile->close();
    }
}

void SoundContainerTmpFileDataLoader::afterProcessing() {
    mTargetCont->setDataLoadedFromTmpFile(mSamples);
}

SoundContainerTmpFileDataSaver::SoundContainerTmpFileDataSaver(
        const stdsptr<Samples> &samples,
        SoundCacheContainer *target) : mTargetCont(target) {
    mSamples = samples;
}

void SoundContainerTmpFileDataSaver::processTask() {
    // mSavingFailed = true; return; // NO TMP FILES !!!
    mTmpFile = qsptr<QTemporaryFile>(new QTemporaryFile());
    if(mTmpFile->open()) {
        mTmpFile->write(rcConstChar(&mSamples->fSampleRange),
                        sizeof(SampleRange));
        const size_t size = static_cast<size_t>(mSamples->fSampleRange.span());
        mTmpFile->write(rcChar(mSamples->fData),
                        static_cast<qint64>(size*sizeof(float)));

        mTmpFile->close();
        mSavingSuccessful = true;
    } else {
        mSavingSuccessful = false;
    }
}

void SoundContainerTmpFileDataSaver::afterProcessing() {
    if(mSavingSuccessful) mTargetCont->setDataSavedToTmpFile(mTmpFile);
}
