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
    mTarget->updateRelativeTransform();
    mTarget->schedulePivotUpdate();
}

PathPointUpdater::PathPointUpdater(PathBox *vectorPath)
{
    mTarget = vectorPath;
}

void PathPointUpdater::update()
{
    mTarget->schedulePathUpdate();
}

GradientUpdater::GradientUpdater(Gradient *gradient)
{
    mTarget = gradient;
}

void GradientUpdater::update()
{
    mTarget->updateQGradientStops();
    //mTarget->scheduleQGradientStopsUpdate();
    //MainWindow::getInstance()->scheduleDisplayedFillStrokeSettingsUpdate();
}

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path)
{
    mTarget = path;
}

void StrokeWidthUpdater::update()
{
    mTarget->scheduleOutlinePathUpdate();
    //MainWindow::getInstance()->scheduleDisplayedFillStrokeSettingsUpdate();
}

DisplayedFillStrokeSettingsUpdater::
DisplayedFillStrokeSettingsUpdater(BoundingBox *path) {
    mTarget = path;
}

void DisplayedFillStrokeSettingsUpdater::update() {
    mTarget->scheduleAwaitUpdate();
    //MainWindow::getInstance()->scheduleDisplayedFillStrokeSettingsUpdate();
}

RectangleBottomRightUpdater::RectangleBottomRightUpdater(Rectangle *target) :
    AnimatorUpdater() {
    mTarget = target;
}

void RectangleBottomRightUpdater::update() {
    mTarget->updateRadiusXAndRange();
}

PixmapEffectUpdater::PixmapEffectUpdater(BoundingBox *target) {
    mTarget = target;
}

void PixmapEffectUpdater::update() {
    mTarget->scheduleEffectsMarginUpdate();
}

#include "Boxes/animationbox.h"
AnimationBoxFrameUpdater::AnimationBoxFrameUpdater(AnimationBox *target) {
    mTarget = target;
}

void AnimationBoxFrameUpdater::update() {
    mTarget->updateAnimationFrame();
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
    mTarget->scheduleAwaitUpdate();
}
