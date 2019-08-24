// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "soundtmpfilehandlers.h"
#include "soundcachecontainer.h"
#include "castmacros.h"

SoundContainerTmpFileDataLoader::SoundContainerTmpFileDataLoader(
        const qsptr<QTemporaryFile> &file,
        SoundCacheContainer *target) : mTargetCont(target) {
    mTmpFile = file;
}

void SoundContainerTmpFileDataLoader::process() {
    if(mTmpFile->open()) {
        SampleRange sampleRange;
        mTmpFile->read(rcChar(&sampleRange), sizeof(SampleRange));
        const size_t size = static_cast<size_t>(sampleRange.span());
        float * data = new float[size];
        mTmpFile->read(rcChar(data),
                       static_cast<qint64>(size*sizeof(float)));
//        mSamples = enve::make_shared<Samples>(data, sampleRange);

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

void SoundContainerTmpFileDataSaver::process() {
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
