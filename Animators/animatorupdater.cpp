#include "Animators/animatorupdater.h"
#include <QDebug>
#include "Boxes/pathbox.h"
#include "mainwindow.h"

TransformUpdater::TransformUpdater(BasicTransformAnimator *transformAnimator) {
    mTarget = transformAnimator;
}

void TransformUpdater::update() {
    mTarget->updateRelativeTransform();
}

void TransformUpdater::updateFinal() {
    mTarget->updateRelativeTransform();
}

void TransformUpdater::frameChangeUpdate() {
    mTarget->updateRelativeTransform();
}

NodePointUpdater::NodePointUpdater(PathBox *vectorPath) {
    mTarget = vectorPath;
}

void NodePointUpdater::update() {
    mTarget->scheduleUpdate();
}

void NodePointUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate();
}

GradientUpdater::GradientUpdater(Gradient *gradient) {
    mTarget = gradient;
}

void GradientUpdater::update() {
    mTarget->updateQGradientStopsFinal();
}

void GradientUpdater::updateFinal() {
    mTarget->updateQGradientStopsFinal();
}

void GradientUpdater::frameChangeUpdate() {
    mTarget->updateQGradientStops();
}

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path) {
    mTarget = path;
}

void StrokeWidthUpdater::update() {
    mTarget->scheduleUpdate();
}

void StrokeWidthUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate();
}

DisplayedFillStrokeSettingsUpdater::
DisplayedFillStrokeSettingsUpdater(BoundingBox *path) {
    mTarget = path;
}

void DisplayedFillStrokeSettingsUpdater::update() {
    mTarget->scheduleUpdate();
}

void DisplayedFillStrokeSettingsUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate();
}

void DisplayedFillStrokeSettingsUpdater::updateFinal() {
    mTarget->clearAllCache();
}

PixmapEffectUpdater::PixmapEffectUpdater(BoundingBox *target) {
    mTarget = target;
}

void PixmapEffectUpdater::update() {
    mTarget->scheduleUpdate();
}

void PixmapEffectUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate();
}

void PixmapEffectUpdater::updateFinal() {
    mTarget->clearAllCache();
}

#include "Boxes/animationbox.h"
AnimationBoxFrameUpdater::AnimationBoxFrameUpdater(AnimationBox *target) {
    mTarget = target;
}

void AnimationBoxFrameUpdater::update() {
    mTarget->scheduleUpdate();
    mTarget->updateDurationRectangleAnimationRange();
}

#include "Boxes/particlebox.h"
ParticlesUpdater::ParticlesUpdater(ParticleEmitter *target) {
    mTarget = target;
}

void ParticlesUpdater::update() {
    mTarget->scheduleGenerateParticles();
}

void GradientPointsUpdater::update() {
    mTarget->scheduleUpdate();
    if(mIsFill) {
        mTarget->updateFillDrawGradient();
    } else {
        mTarget->updateStrokeDrawGradient();
    }
    mTarget->scheduleUpdate();
}

void GradientPointsUpdater::frameChangeUpdate() {
    if(mIsFill) {
        mTarget->updateFillDrawGradient();
    } else {
        mTarget->updateStrokeDrawGradient();
    }
    mTarget->scheduleUpdate();
}

#include "Sound/singlesound.h"
SingleSoundUpdater::SingleSoundUpdater(SingleSound *sound) : AnimatorUpdater() {
    mTarget = sound;
}

SingleSoundUpdater::~SingleSoundUpdater() {
}

void SingleSoundUpdater::update() {
    mTarget->scheduleFinalDataUpdate();
}
#include "Boxes/paintbox.h"
PaintBoxSizeUpdaterBR::PaintBoxSizeUpdaterBR(PaintBox *paintBox) {
    mTarget = paintBox;
}

PaintBoxSizeUpdaterBR::~PaintBoxSizeUpdaterBR() {
}

void PaintBoxSizeUpdaterBR::update() {
    mTarget->scheduleUpdate();
}

void PaintBoxSizeUpdaterBR::frameChangeUpdate() {
    //mTarget->scheduleUpdate();
}

void PaintBoxSizeUpdaterBR::updateFinal() {
    mTarget->scheduleFinishSizeSetup();
    mTarget->scheduleUpdate();
}

PaintBoxSizeUpdaterTL::PaintBoxSizeUpdaterTL(PaintBox *paintBox) {
    mTarget = paintBox;
}

PaintBoxSizeUpdaterTL::~PaintBoxSizeUpdaterTL() {
}

void PaintBoxSizeUpdaterTL::update() {
    mTarget->scheduleUpdate();
}

void PaintBoxSizeUpdaterTL::frameChangeUpdate() {
    //mTarget->scheduleUpdate();
}

void PaintBoxSizeUpdaterTL::updateFinal() {
    mTarget->scheduleFinishSizeAndPosSetup();
    mTarget->scheduleUpdate();
}

RandomQrealGeneratorUpdater::RandomQrealGeneratorUpdater(
        RandomQrealGenerator *target) {
    mTarget = target;
}
#include "randomqrealgenerator.h"
void RandomQrealGeneratorUpdater::update() {
    mTarget->generateData();
}
