#ifndef BLUREFFECT_H
#define BLUREFFECT_H
#include "rastereffect.h"

class BlurEffect : public RasterEffect {
    e_OBJECT
protected:
    BlurEffect();
public:
    stdsptr<RasterEffectCaller> getEffectCaller(
            const qreal relFrame, const qreal resolution,
            const qreal influence) const;
    QMargins getMargin() const;
    bool forceMargin() const { return true; }
private:
    qsptr<QrealAnimator> mRadius;
};

#endif // BLUREFFECT_H
