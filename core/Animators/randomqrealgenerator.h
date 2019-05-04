#ifndef RANDOMQREALGENERATOR_H
#define RANDOMQREALGENERATOR_H
#include "qrealvalueeffect.h"
class IntProperty;
class IntAnimator;
class ComboBoxProperty;

class RandomQrealGenerator : public QrealValueEffect {
    friend class SelfRef;
protected:
    RandomQrealGenerator();
public:
    qreal getDevAtRelFrame(const qreal &relFrame);
    void anim_setAbsFrame(const int &frame);
    FrameRange prp_getIdenticalRelRange(const int &relFrame) const;

    void writeProperty(QIODevice * const device) const;
    void readProperty(QIODevice *device);
private:
    qsptr<QrealAnimator> mTime;
    qsptr<QrealAnimator> mSmoothness;
    qsptr<QrealAnimator> mMaxDev;
    qsptr<IntAnimator> mSeedAssist;
    qsptr<ComboBoxProperty> mType;
};

#endif // RANDOMQREALGENERATOR_H
