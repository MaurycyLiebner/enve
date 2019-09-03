#ifndef ESOUND_H
#define ESOUND_H
#include "Animators/eboxorsound.h"
#include "Animators/qrealanimator.h"
class Samples;
class SoundReaderForMerger;

class eSound : public eBoxOrSound {
public:
    eSound();

    bool SWT_isSound() const { return true; }

    virtual int durationSeconds() const = 0;
    virtual QrealSnapshot getVolumeSnap() const = 0;
    virtual stdsptr<Samples> getSamplesForSecond(const int relSecondId) = 0;
    virtual SoundReaderForMerger * getSecondReader(const int relSecondId) = 0;
    virtual qreal getStretch() const = 0;
    virtual qsptr<eSound> createLink() = 0;

    iValueRange absSecondToRelSeconds(const int absSecond);
    int getSampleShift() const;
    SampleRange relSampleRange() const;
    SampleRange absSampleRange() const;
protected:
    qreal getCanvasFPS() const;
private:
    iValueRange absSecondToRelSecondsAbsStretch(const int absSecond);
};

#endif // ESOUND_H
