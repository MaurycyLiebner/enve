#ifndef SINGLESOUND_H
#define SINGLESOUND_H
#include "Animators/complexanimator.h"

class SingleSound : public ComplexAnimator
{
public:
    SingleSound();
    SingleSound(const QString &path);

    void setFilePath(const QString &path);
    void reloadDataFromFile();

    int getStartFrame() const;
    int getSampleCount() const;
    const float *getFinalData() const;
    void prepareFinalData();
private:
    float *mSrcData = NULL;
    float *mFinalData = NULL;
    int mStartFrame = 0;
    int mFinalSampleCount = 0;
    int mSrcSampleCount = 0;
    QrealAnimator mVolumeAnimator;
    QString mPath;
};

#endif // SINGLESOUND_H
