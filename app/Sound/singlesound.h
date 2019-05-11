#ifndef SINGLESOUND_H
#define SINGLESOUND_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
#include "CacheHandlers/soundcachehandler.h"
class FixedLenAnimationRect;

class SingleSound : public ComplexAnimator {
    friend class SelfRef;
protected:
    SingleSound(const qsptr<FixedLenAnimationRect> &durRect = nullptr);
public:
    bool SWT_isSingleSound() const { return true; }

    DurationRectangleMovable *anim_getTimelineMovable(
            const int &relX, const int &minViewedFrame,
            const qreal &pixelsPerFrame);
    void drawTimelineControls(QPainter * const p,
                              const qreal &pixelsPerFrame,
                              const FrameRange &absFrameRange,
                              const int &rowHeight);

    int prp_getRelFrameShift() const;

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget) const;

    FrameRange prp_relInfluenceRange() const;

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice * const target);

    void setFilePath(const QString &path);

    void setDurationRect(const qsptr<FixedLenAnimationRect> &durRect);
    FixedLenAnimationRect *getDurationRect() const;

    qreal getVolumeAtRelFrame(const qreal& relFrame) const {
        return mVolumeAnimator->getEffectiveValue(relFrame);
    }

    SoundReaderForMerger * getSecondReader(const int& relSecondId) {
        const int maxSec = mCacheHandler->durationSec() - 1;
        if(relSecondId < 0 || relSecondId > maxSec) return nullptr;
        const auto reader = mCacheHandler->getSecondReader(relSecondId);
        if(!reader) return mCacheHandler->addSecondReader(relSecondId);
        return reader;
    }

    stdsptr<Samples> getSamplesForSecond(const int& relSecondId) {
        return mCacheHandler->getSamplesForSecond(relSecondId);
    }

    int getSampleShift() const;
    SampleRange relSampleRange() const;
    SampleRange absSampleRange() const;
    iValueRange absSecondToRelSeconds(const int& absSecond);

    const HDDCachableCacheHandler* getCacheHandler() const {
        if(!mCacheHandler) return nullptr;
        return &mCacheHandler->getCacheHandler();
    }

    qreal getSpeed() const { return 1; }
    QrealSnapshot getVolumeSnap() const {
        return mVolumeAnimator->makeSnapshot(
                    SOUND_SAMPLERATE/getCanvasFPS(), 0.01);
    }
private:
    qreal getCanvasFPS() const;
    void updateAfterDurationRectangleShifted();
    bool mOwnDurationRectangle;

    stdptr<SoundCacheHandler> mCacheHandler;

    qsptr<FixedLenAnimationRect> mDurationRectangle;

    qsptr<QrealAnimator> mVolumeAnimator =
            SPtrCreate(QrealAnimator)(100, 0, 200, 1, "volume");
};

#endif // SINGLESOUND_H
