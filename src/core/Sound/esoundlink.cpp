#include "esoundlink.h"

eSoundLink::eSoundLink(eSound * const target) :
    mTarget(target) {
    connect(mTarget, &eBoxOrSound::visibilityChanged,
            this, &eBoxOrSound::setVisibile);
    connect(mTarget, &eSound::prp_absFrameRangeChanged,
            this, [this](const FrameRange& range) {
        const auto relRange = mTarget->prp_absRangeToRelRange(range);
        prp_afterChangedRelRange(relRange);
    });
}

FrameRange eSoundLink::prp_relInfluenceRange() const {
    return mTarget->prp_relInfluenceRange();
}

int eSoundLink::prp_getRelFrameShift() const {
    return mTarget->prp_getRelFrameShift();
}

int eSoundLink::durationSeconds() const {
    return mTarget->durationSeconds();
}

QrealSnapshot eSoundLink::getVolumeSnap() const {
    return mTarget->getVolumeSnap();
}

stdsptr<Samples> eSoundLink::getSamplesForSecond(const int relSecondId) {
    return mTarget->getSamplesForSecond(relSecondId);
}

SoundReaderForMerger * eSoundLink::getSecondReader(const int relSecondId) {
    return mTarget->getSecondReader(relSecondId);
}

qreal eSoundLink::getStretch() const {
    return mTarget->getStretch();
}
