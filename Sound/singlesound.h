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
public:
    SingleSound(const QString &path, DurationRectangle *durRect = NULL);

    void setDurationRect(DurationRectangle *durRect);

    void setFilePath(const QString &path);
    void reloadDataFromFile();

    int getStartFrame() const;
    int getSampleCount() const;
    const float *getFinalData() const;
    void prepareFinalData();
    void drawKeys(QPainter *p,
                  qreal pixelsPerFrame,
                  qreal drawY,
                  int startFrame, int endFrame);
    DurationRectangleMovable *getRectangleMovableAtPos(qreal relX,
                                                       int minViewedFrame,
                                                       qreal pixelsPerFrame);
    SWT_Type SWT_getType() { return SWT_SingleSound; }
private:
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
