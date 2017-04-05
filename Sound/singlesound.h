#ifndef SINGLESOUND_H
#define SINGLESOUND_H
#include "Animators/complexanimator.h"
class DurationRectangle;

extern int decode_audio_file(const char* path,
                             const int sample_rate,
                             float** audioData,
                             int* size);

class SingleSound : public ComplexAnimator
{
    Q_OBJECT
public:
    SingleSound(const QString &path, DurationRectangle *durRect = NULL);

    void setDurationRect(DurationRectangle *durRect);

    void setFilePath(const QString &path);
    void reloadDataFromFile();

    int getStartAbsFrame() const;
    int getSampleCount() const;
    const float *getFinalData() const;
    void prepareFinalData(const qreal &fps, const int &minAbsFrame, const int &maxAbsFrame);
    void prp_drawKeys(QPainter *p,
                  qreal pixelsPerFrame,
                  qreal drawY,
                  int startFrame, int endFrame);
    DurationRectangleMovable *anim_getRectangleMovableAtPos(qreal relX,
                                                       int minViewedFrame,
                                                       qreal pixelsPerFrame);
    SWT_Type SWT_getType() { return SWT_SingleSound; }
    void updateFinalDataIfNeeded(const qreal &fps,
                                 const int &minAbsFrame,
                                 const int &maxAbsFrame);
    int prp_getFrameShift() const;

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);
public slots:
    void scheduleFinalDataUpdate();
private:
    bool mFinalDataUpdateNeeded = false;
    bool mOwnDurationRectangle;
    DurationRectangle *mDurationRectangle = NULL;
    float *mSrcData = NULL;
    float *mFinalData = NULL;
    int mFinalSampleCount = 0;
    int mSrcSampleCount = 0;
    QrealAnimator mVolumeAnimator;
    QString mPath;
};

#endif // SINGLESOUND_H
