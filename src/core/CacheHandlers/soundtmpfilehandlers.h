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

#ifndef SOUNDTMPFILEHANDLERS_H
#define SOUNDTMPFILEHANDLERS_H
#include "tmpfilehandlers.h"
#include "soundcachecontainer.h"
#include "Tasks/updatable.h"
#include <QTemporaryFile>
#include "skia/skiaincludes.h"
class SoundCacheContainer;
class SoundContainerTmpFileDataLoader : public eHddTask {
    e_OBJECT
public:
    void process();

    void afterProcessing();
protected:
    SoundContainerTmpFileDataLoader(const qsptr<QTemporaryFile> &file,
                                    SoundCacheContainer *target);

    const stdptr<SoundCacheContainer> mTargetCont;
    qsptr<QTemporaryFile> mTmpFile;
    stdsptr<Samples> mSamples;
};

class SoundContainerTmpFileDataSaver : public eHddTask {
    e_OBJECT
public:
    void process();

    void afterProcessing();
protected:
    SoundContainerTmpFileDataSaver(const stdsptr<Samples> &samples,
                                   SoundCacheContainer *target);

    bool mSavingSuccessful = false;
    const stdptr<SoundCacheContainer> mTargetCont;
    stdsptr<Samples> mSamples;
    qsptr<QTemporaryFile> mTmpFile;
};

#endif // SOUNDTMPFILEHANDLERS_H
