#ifndef RANDOMQREALGENERATOR_H
#define RANDOMQREALGENERATOR_H
#include "qrealvalueeffect.h"
class IntProperty;
class ComboBoxProperty;

struct FrameValue {
    FrameValue(const qreal &frameT, const qreal &valueT) {
        frame = frameT;
        value = valueT;
    }
    qreal frame;
    qreal value;
};

class RandomQrealGenerator : public QrealValueEffect {
    friend class SelfRef;
public:
    qreal getDevAtRelFrame(const qreal &relFrame);
    void generateData();
    void anim_setAbsFrame(const int &frame);
    FrameRange prp_getIdenticalRelRange(const int &relFrame) const;

    void writeProperty(QIODevice * const device) const;
    void readProperty(QIODevice *device);
protected:
    RandomQrealGenerator(const int &firstFrame, const int &lastFrame);

    int getClosestLowerFrameId(const int &minId,
                               const int &maxId,
                               const int &targetFrame);
    qreal getDeltaX(const int &relFrame);
private:
    int mFirstFrame;
    int mLastFrame;
    QList<FrameValue> mFrameValues;
    qsptr<QrealAnimator> mPeriod;
    qsptr<QrealAnimator> mSmoothness;
    qsptr<QrealAnimator> mMaxDev;
    qsptr<IntProperty> mSeedAssist;
    qsptr<ComboBoxProperty> mType;
};

#endif // RANDOMQREALGENERATOR_H
