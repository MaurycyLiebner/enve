#include "Animators/animatorupdater.h"
#include <QDebug>
#include "Boxes/pathbox.h"
#include "mainwindow.h"
#include "Boxes/boxesgroup.h"

TransformUpdater::TransformUpdater(BasicTransformAnimator *transformAnimator) {
    mTarget = transformAnimator;
}

void TransformUpdater::update() {
    mTarget->updateRelativeTransform(Animator::USER_CHANGE);
}

void TransformUpdater::updateFinal() {
    mTarget->updateRelativeTransform(Animator::USER_CHANGE);
}

void TransformUpdater::frameChangeUpdate() {
    mTarget->updateRelativeTransform(Animator::FRAME_CHANGE);
}

NodePointUpdater::NodePointUpdater(PathBox *vectorPath) {
    mTarget = vectorPath;
}

void NodePointUpdater::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
    mTarget->setPathsOutdated(true);
}

void NodePointUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate(Animator::FRAME_CHANGE);
}

GradientUpdater::GradientUpdater(Gradient *gradient) {
    mTarget = gradient;
}

void GradientUpdater::update() {
    mTarget->updateQGradientStopsFinal(Animator::USER_CHANGE);
}

void GradientUpdater::updateFinal() {
    mTarget->updateQGradientStopsFinal(Animator::USER_CHANGE);
}

void GradientUpdater::frameChangeUpdate() {
    mTarget->updateQGradientStops(Animator::FRAME_CHANGE);
}

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path) {
    mTarget = path;
}

void StrokeWidthUpdater::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void StrokeWidthUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate(Animator::FRAME_CHANGE);
}

DisplayedFillStrokeSettingsUpdater::
DisplayedFillStrokeSettingsUpdater(BoundingBox *path) {
    mTarget = path;
}

void DisplayedFillStrokeSettingsUpdater::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void DisplayedFillStrokeSettingsUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate(Animator::FRAME_CHANGE);
}

void DisplayedFillStrokeSettingsUpdater::updateFinal() {
    mTarget->clearAllCache();
}

PixmapEffectUpdater::PixmapEffectUpdater(BoundingBox *target) {
    mTarget = target;
}

void PixmapEffectUpdater::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void PixmapEffectUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate(Animator::FRAME_CHANGE);
}

void PixmapEffectUpdater::updateFinal() {
    mTarget->clearAllCache();
}

#include "Boxes/animationbox.h"
AnimationBoxFrameUpdater::AnimationBoxFrameUpdater(AnimationBox *target) {
    mTarget = target;
}

void AnimationBoxFrameUpdater::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
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
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
    if(mIsFill) {
        mTarget->updateFillDrawGradient();
    } else {
        mTarget->updateStrokeDrawGradient();
    }
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void GradientPointsUpdater::frameChangeUpdate() {
    if(mIsFill) {
        mTarget->updateFillDrawGradient();
    } else {
        mTarget->updateStrokeDrawGradient();
    }
    mTarget->scheduleUpdate(Animator::FRAME_CHANGE);
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
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void PaintBoxSizeUpdaterBR::frameChangeUpdate() {
    //mTarget->scheduleUpdate();
}

void PaintBoxSizeUpdaterBR::updateFinal() {
    mTarget->scheduleFinishSizeSetup();
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

PaintBoxSizeUpdaterTL::PaintBoxSizeUpdaterTL(PaintBox *paintBox) {
    mTarget = paintBox;
}

PaintBoxSizeUpdaterTL::~PaintBoxSizeUpdaterTL() {
}

void PaintBoxSizeUpdaterTL::update() {
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

void PaintBoxSizeUpdaterTL::frameChangeUpdate() {
    //mTarget->scheduleUpdate();
}

void PaintBoxSizeUpdaterTL::updateFinal() {
    mTarget->scheduleFinishSizeAndPosSetup();
    mTarget->scheduleUpdate(Animator::USER_CHANGE);
}

RandomQrealGeneratorUpdater::RandomQrealGeneratorUpdater(
        RandomQrealGenerator *target) {
    mTarget = target;
}
#include "randomqrealgenerator.h"
void RandomQrealGeneratorUpdater::update() {
    mTarget->generateData();
}

GroupAllPathsUpdater::GroupAllPathsUpdater(BoxesGroup *boxesGroup) {
    mTarget = boxesGroup;
}

void GroupAllPathsUpdater::update() {
    mTarget->updateAllChildPathBoxes(Animator::USER_CHANGE);
}

void GroupAllPathsUpdater::frameChangeUpdate() {
    mTarget->updateAllChildPathBoxes(Animator::FRAME_CHANGE);
}

AnimatorUpdater::AnimatorUpdater() {}

AnimatorUpdater::~AnimatorUpdater() {}

void AnimatorUpdater::update() {}

void AnimatorUpdater::frameChangeUpdate() {
    update();
}

void AnimatorUpdater::updateFinal() {}
