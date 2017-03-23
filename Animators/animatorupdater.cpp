#include "Animators/animatorupdater.h"
#include <QDebug>
#include "Boxes/pathbox.h"
#include "mainwindow.h"
#include "Boxes/circle.h"
#include "Boxes/rectangle.h"

TransUpdater::TransUpdater(BoundingBox *boundingBox) : AnimatorUpdater()
{
    mTarget = boundingBox;
}

void TransUpdater::update() {
    mTarget->updateRelativeTransformTmp();
    mTarget->schedulePivotUpdate();
}

void TransUpdater::updateFinal() {
    mTarget->updateCombinedTransform();
}

void TransUpdater::frameChangeUpdate() {
    mTarget->updateRelativeTransformAfterFrameChange();
    mTarget->schedulePivotUpdate();
}

PathPointUpdater::PathPointUpdater(PathBox *vectorPath) {
    mTarget = vectorPath;
}

void PathPointUpdater::update() {
    mTarget->schedulePathUpdate();
}

void PathPointUpdater::frameChangeUpdate() {
    mTarget->schedulePathUpdate();
}

GradientUpdater::GradientUpdater(Gradient *gradient) {
    mTarget = gradient;
}

void GradientUpdater::update() {
    mTarget->updateQGradientStops();
}

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path) {
    mTarget = path;
}

void StrokeWidthUpdater::update() {
    mTarget->scheduleOutlinePathUpdate();
}

void StrokeWidthUpdater::frameChangeUpdate() {
    mTarget->scheduleOutlinePathUpdate();
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

PixmapEffectUpdater::PixmapEffectUpdater(BoundingBox *target) {
    mTarget = target;
}

void PixmapEffectUpdater::update() {
    mTarget->scheduleEffectsMarginUpdate();
}

void PixmapEffectUpdater::frameChangeUpdate() {
    mTarget->scheduleEffectsMarginUpdate();
}

#include "Boxes/animationbox.h"
AnimationBoxFrameUpdater::AnimationBoxFrameUpdater(AnimationBox *target) {
    mTarget = target;
}

void AnimationBoxFrameUpdater::update() {
    mTarget->updateDurationRectanglePossibleRange();
}

#include "Boxes/particlebox.h"
ParticlesUpdater::ParticlesUpdater(ParticleEmitter *target) {
    mTarget = target;
}

void ParticlesUpdater::update() {
    mTarget->scheduleGenerateParticles();
}

void GradientPointsUpdater::update() {
    if(mIsFill) {
        mTarget->updateFillDrawGradient();
    } else {
        mTarget->updateStrokeDrawGradient();
    }
    mTarget->scheduleUpdate();
}

void GradientPointsUpdater::frameChangeUpdate() {
    mTarget->scheduleUpdate();
}

#include "Sound/singlesound.h"
SingleSoundUpdater::SingleSoundUpdater(SingleSound *sound) : AnimatorUpdater() {
    mTarget = sound;
    mTarget->incNumberPointers();
}

SingleSoundUpdater::~SingleSoundUpdater() {
    mTarget->decNumberPointers();
}

void SingleSoundUpdater::update() {
    mTarget->scheduleFinalDataUpdate();
}
