#ifndef ESOUNDLINK_H
#define ESOUNDLINK_H
#include "esound.h"

class eSoundLink : public eSound {
public:
    eSoundLink(eSound* const target);

    qsptr<eSound> createLink() {
        return enve::make_shared<eSoundLink>(mTarget);
    }

    FrameRange prp_relInfluenceRange() const;
    int prp_getRelFrameShift() const;

    int durationSeconds() const;
    QrealSnapshot getVolumeSnap() const;
    stdsptr<Samples> getSamplesForSecond(const int relSecondId);
    SoundReaderForMerger * getSecondReader(const int relSecondId);
    qreal getStretch() const;
private:
    eSound* const mTarget;
};

#endif // ESOUNDLINK_H
