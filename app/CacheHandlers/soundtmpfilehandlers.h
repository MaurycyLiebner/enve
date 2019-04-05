#ifndef SOUNDTMPFILEHANDLERS_H
#define SOUNDTMPFILEHANDLERS_H
#include "tmpfilehandlers.h"
#include "soundcachecontainer.h"
#include "updatable.h"
#include <QTemporaryFile>
#include "skia/skiaincludes.h"
class SoundCacheContainer;
class SoundContainerTmpFileDataLoader : public _HDDTask {
    friend class StdSelfRef;
public:
    void _processUpdate();

    void afterProcessingFinished();
protected:
    SoundContainerTmpFileDataLoader(const qsptr<QTemporaryFile> &file,
                                    SoundCacheContainer *target);

    const stdptr<SoundCacheContainer> mTargetCont;
    qsptr<QTemporaryFile> mTmpFile;
    stdsptr<Samples> mSamples;
};

class SoundContainerTmpFileDataSaver : public _HDDTask {
    friend class StdSelfRef;
public:
    void _processUpdate();

    void afterProcessingFinished();
protected:
    SoundContainerTmpFileDataSaver(const stdsptr<Samples> &samples,
                                   SoundCacheContainer *target);

    bool mSavingSuccessful = false;
    const stdptr<SoundCacheContainer> mTargetCont;
    stdsptr<Samples> mSamples;
    qsptr<QTemporaryFile> mTmpFile;
};

#endif // SOUNDTMPFILEHANDLERS_H
