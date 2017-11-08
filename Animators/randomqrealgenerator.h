#ifndef RANDOMQREALGENERATOR_H
#define RANDOMQREALGENERATOR_H
#include "qrealvalueeffect.h"
class QrealAnimator;
typedef QSharedPointer<QrealAnimator> QrealAnimatorQSPtr;
class ComboBoxProperty;
typedef QSharedPointer<ComboBoxProperty> ComboBoxPropertyQSPtr;
class IntProperty;
typedef QSharedPointer<IntProperty> IntPropertyQSPtr;

struct FrameValue {
    FrameValue(const qreal &frameT, const qreal &valueT) {
        frame = frameT;
        value = valueT;
    }
    qreal frame;
    qreal value;
};

class RandomQrealGenerator : public QrealValueEffect {
public:
    RandomQrealGenerator(const int &firstFrame, const int &lastFrame);
    qreal getDevAtRelFrame(const int &relFrame);
    void generateData();
    void prp_setAbsFrame(const int &frame);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);

    void writeProperty(QIODevice *device);
    void readProperty(QIODevice *device);
protected:
    int getClosestLowerFrameId(const int &minId,
                               const int &maxId,
                               const int &targetFrame);
    qreal getDeltaX(const int &relFrame);
private:
    int mFirstFrame;
    int mLastFrame;
    QList<FrameValue> mFrameValues;
    QrealAnimatorQSPtr mPeriod;
    QrealAnimatorQSPtr mSmoothness;
    QrealAnimatorQSPtr mMaxDev;
    IntPropertyQSPtr mSeedAssist;
    ComboBoxPropertyQSPtr mType;
};

#endif // RANDOMQREALGENERATOR_H
