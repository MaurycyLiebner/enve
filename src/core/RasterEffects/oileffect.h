#ifndef OILEFFECT_H
#define OILEFFECT_H

#include "rastereffect.h"

class CORE_EXPORT OilEffect : public RasterEffect {
    e_OBJECT
private:
    OilEffect();
public:
    QMargins getMargin() const;

    stdsptr<RasterEffectCaller> getEffectCaller(
            const qreal relFrame, const qreal resolution,
            const qreal influence, BoxRenderData* const data) const;
};

#endif // OILEFFECT_H
