#ifndef SINGLESOUND_H
#define SINGLESOUND_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
class FixedLenAnimationRect;

class SingleSound : public ComplexAnimator {
    friend class SelfRef;
protected:
    SingleSound(const QString &path,
                const qsptr<FixedLenAnimationRect> &durRect = nullptr);
public:
    bool SWT_isSingleSound() const { return true; }

    void drawTimelineControls(QPainter * const p,
                              const qreal &pixelsPerFrame,
                              const FrameRange &absFrameRange,
                              const int &rowHeight);
    DurationRectangleMovable *anim_getRectangleMovableAtPos(
            const int &relX, const int &minViewedFrame,
            const qreal &pixelsPerFrame);

    int prp_getFrameShift() const;

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget) const;
    void setFilePath(const QString &path);
    void reloadDataFromFile();

    int getSampleRate() const {
        return mSampleRate;
    }

    int getStartAbsFrame() const;
    int getSampleCount() const;
    const float *getFinalData() const;
    void prepareFinalData(const float &fps,
                          const int &minAbsFrame,
                          const int &maxAbsFrame);

    void scheduleFinalDataUpdate();
    void updateFinalDataIfNeeded(const qreal &fps,
                                 const int &minAbsFrame,
                                 const int &maxAbsFrame);

    void setDurationRect(const qsptr<FixedLenAnimationRect> &durRect);
    FixedLenAnimationRect *getDurationRect();

    qreal getVolumeAtRelFrame(const qreal& relFrame) const {
        return mVolumeAnimator->getEffectiveValue(relFrame);
    }
private:
    void updateAfterDurationRectangleShifted();

    bool mFinalDataUpdateNeeded = false;
    bool mOwnDurationRectangle;

    int mFinalAbsStartFrame = 0;
    int mSrcSampleCount = 0;
    int mFinalSampleCount = 0;

    QString mPath;

    int mSampleRate = 44100;

    float *mSrcData = nullptr;
    float *mFinalData = nullptr;

    qsptr<FixedLenAnimationRect> mDurationRectangle;

    qsptr<QrealAnimator> mVolumeAnimator =
            SPtrCreate(QrealAnimator)(100, 0, 200, 1, "volume");
};

#endif // SINGLESOUND_H
