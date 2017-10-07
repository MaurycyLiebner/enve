#ifndef SINGLESOUND_H
#define SINGLESOUND_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimator.h"
class FixedLenAnimationRect;

extern int decode_audio_file(const char* path,
                             const int sample_rate,
                             float** audioData,
                             int* size);

class SingleSound : public ComplexAnimator
{
    Q_OBJECT
public:
    SingleSound(const QString &path,
                FixedLenAnimationRect *durRect = NULL);

    void setDurationRect(FixedLenAnimationRect *durRect);

    void setFilePath(const QString &path);
    void reloadDataFromFile();

    int getStartAbsFrame() const;
    int getSampleCount() const;
    const float *getFinalData() const;
    void prepareFinalData(const qreal &fps,
                          const int &minAbsFrame,
                          const int &maxAbsFrame);
    void prp_drawKeys(QPainter *p,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame,
                      const int &endFrame);
    DurationRectangleMovable *anim_getRectangleMovableAtPos(
                                            const qreal &relX,
                                            const int &minViewedFrame,
                                            const qreal &pixelsPerFrame);
    void updateFinalDataIfNeeded(const qreal &fps,
                                 const int &minAbsFrame,
                                 const int &maxAbsFrame);
    int prp_getFrameShift() const;

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);
    FixedLenAnimationRect *getDurationRect();
public slots:
    void scheduleFinalDataUpdate();
private slots:
    void updateAfterDurationRectangleShifted();
private:
    bool mFinalDataUpdateNeeded = false;
    bool mOwnDurationRectangle;
    FixedLenAnimationRect *mDurationRectangle = NULL;
    float *mSrcData = NULL;
    float *mFinalData = NULL;
    int mFinalSampleCount = 0;
    int mFinalAbsStartFrame = 0;
    int mSrcSampleCount = 0;
    QSharedPointer<QrealAnimator> mVolumeAnimator =
            (new QrealAnimator())->ref<QrealAnimator>();
    QString mPath;
};

#endif // SINGLESOUND_H
