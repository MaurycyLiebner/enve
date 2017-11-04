#ifndef RANDOMQREALGENERATOR_H
#define RANDOMQREALGENERATOR_H
#include "complexanimator.h"
class QrealAnimator;
typedef QSharedPointer<QrealAnimator> QrealAnimatorQSPtr;

struct FrameValue {
    FrameValue(const qreal &frameT, const qreal &valueT) {
        frame = frameT;
        value = valueT;
    }
    qreal frame;
    qreal value;
};

class RandomQrealGenerator : public ComplexAnimator {
public:
    RandomQrealGenerator(const int &firstFrame, const int &lastFrame);
    qreal getDevAtRelFrame(const int &relFrame);
    void generateData();
    void prp_setAbsFrame(const int &frame);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
protected:
    int getClosestLowerFrameId(const int &minId,
                               const int &maxId,
                               const int &targetFrame);
    qreal getDeltaX(const int &relFrame);
private:
    int mFirstFrame;
    int mLastFrame;
    uint32_t mSeedAssist = 0;
    QList<FrameValue> mFrameValues;
    QrealAnimatorQSPtr mPeriod;
    QrealAnimatorQSPtr mSmoothness;
    QrealAnimatorQSPtr mMaxDev;
};

#endif // RANDOMQREALGENERATOR_H
