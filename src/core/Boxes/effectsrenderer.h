#ifndef EFFECTSRENDERER_H
#define EFFECTSRENDERER_H
#include "smartPointers/ememory.h"
#include "glhelpers.h"
#include "Tasks/updatable.h"

class RasterEffectCaller;
struct BoxRenderData;

class EffectsRenderer {
public:
    void add(const stdsptr<RasterEffectCaller>& effect) {
        mEffects.append(effect);
    }

    void processGpu(QGL33 * const gl,
                    SwitchableContext &context,
                    BoxRenderData * const boxData);
    void processCpu(BoxRenderData * const boxData);

    bool isEmpty() const { return mEffects.isEmpty(); }

    void setBaseGlobalRect(SkIRect& currRect,
                           const SkIRect& skMaxBounds) const;

    HardwareSupport nextHardwareSupport() const;
private:
    QList<stdsptr<RasterEffectCaller>> mEffects;
};
#endif // EFFECTSRENDERER_H
