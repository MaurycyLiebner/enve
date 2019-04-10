#include "gpueffectanimators.h"
#include "GPUEffects/gpurastereffect.h"
#include "Boxes/boundingbox.h"
#include <QDebug>

GPUEffectAnimators::GPUEffectAnimators(BoundingBox *parentBox) :
    ComplexAnimator("gpu effects"), mParentBox_k(parentBox) {}

void GPUEffectAnimators::addEffect(const qsptr<GPURasterEffect>& effect) {
    mParentBox_k->addGPUEffect(effect);
}

qreal GPUEffectAnimators::getEffectsMargin() const {
    qreal newMargin = 2.;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), GPURasterEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMargin();
        }
    }
    return newMargin;
}

qreal GPUEffectAnimators::getEffectsMarginAtRelFrame(const int &relFrame) const {
    qreal newMargin = 0.;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), GPURasterEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(relFrame);
        }
    }
    return newMargin;
}

qreal GPUEffectAnimators::getEffectsMarginAtRelFrameF(const qreal &relFrame) const {
    qreal newMargin = 0.;
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect.get(), GPURasterEffect);
        if(pixmapEffect->isVisible()) {
            newMargin += pixmapEffect->getMarginAtRelFrame(qRound(relFrame));
        }
    }
    return newMargin;
}

void GPUEffectAnimators::addEffectRenderDataToListF(
        const qreal &relFrame,
        BoundingBoxRenderData * const data) {
    for(const auto& effect : ca_mChildAnimators) {
        auto pixmapEffect = GetAsPtr(effect, GPURasterEffect);
        if(pixmapEffect->isVisible()) {
            stdsptr<GPURasterEffectCaller> effectRenderData =
                    pixmapEffect->getGPURasterEffectCaller(relFrame);
            if(!effectRenderData) continue;
            data->fGPUEffects.append(effectRenderData);
        }
    }
}

void GPUEffectAnimators::ca_removeAllChildAnimators() {
    ComplexAnimator::ca_removeAllChildAnimators();
    mParentBox_k->ca_removeChildAnimator(ref<BoundingBox>());
}

bool GPUEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}
