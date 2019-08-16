#ifndef SINGLESOUND_H
#define SINGLESOUND_H
#include "Animators/eboxorsound.h"
#include "Animators/qrealanimator.h"
class FixedLenAnimationRect;
class SoundHandler;
class Samples;
class SoundReaderForMerger;
class SoundDataHandler;
class HDDCachableCacheHandler;

class SingleSound : public eBoxOrSound {
    e_OBJECT
    Q_OBJECT
protected:
    SingleSound(const qsptr<FixedLenAnimationRect> &durRect = nullptr);
public:
    bool SWT_isSound() const { return true; }
    bool SWT_isSingleSound() const { return mOwnDurationRectangle; }

    void setupTreeViewMenu(PropertyMenu * const menu);

    int prp_getRelFrameShift() const;

    QMimeData *SWT_createMimeData() {
        if(!mOwnDurationRectangle) return nullptr;
        return eBoxOrSound::SWT_createMimeData();
    }

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool parentSatisfies,
                             const bool parentMainTarget) const;

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice * const target);

    void setFilePath(const QString &path);

    qreal getVolumeAtRelFrame(const qreal relFrame) const {
        return mVolumeAnimator->getEffectiveValue(relFrame);
    }

    SoundReaderForMerger * getSecondReader(const int relSecondId);

    stdsptr<Samples> getSamplesForSecond(const int relSecondId);

    int getSampleShift() const;
    SampleRange relSampleRange() const;
    SampleRange absSampleRange() const;
    iValueRange absSecondToRelSeconds(const int absSecond);

    const HDDCachableCacheHandler* getCacheHandler() const;

    bool videoSound() const {
        return !mOwnDurationRectangle;
    }

    void setStretch(const qreal stretch);
    qreal getStretch() const { return mStretch; }
    QrealSnapshot getVolumeSnap() const;

    bool isEnabled() const {
        return mEnabled;
    }

    void setEnabled(const bool enable) {
        if(enable == mEnabled) return;
        mEnabled = enable;
        SWT_setDisabled(!mEnabled);
        prp_afterWholeInfluenceRangeChanged();
    }
    void setSoundDataHandler(SoundDataHandler * const newDataHandler);
private:
    iValueRange absSecondToRelSecondsAbsStretch(const int absSecond);
    void updateDurationRectLength();

    qreal getCanvasFPS() const;
    const bool mOwnDurationRectangle;

    bool mEnabled = true;
    qreal mStretch = 1;
    stdsptr<SoundHandler> mCacheHandler;

    qsptr<QrealAnimator> mVolumeAnimator =
            enve::make_shared<QrealAnimator>(100, 0, 200, 1, "volume");
};

#endif // SINGLESOUND_H
