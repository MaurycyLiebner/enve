#include "animatorupdater.h"
#include <QDebug>
#include "pathbox.h"
#include "mainwindow.h"
#include "circle.h"
#include "rectangle.h"

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
    mTarget->scheduleQGradientStopsUpdate();
    MainWindow::getInstance()->scheduleDisplayedFillStrokeSettingsUpdate();
}

StrokeWidthUpdater::StrokeWidthUpdater(PathBox *path)
{
    mTarget = path;
}

void StrokeWidthUpdater::update()
{
    mTarget->scheduleOutlinePathUpdate();
    MainWindow::getInstance()->scheduleDisplayedFillStrokeSettingsUpdate();
}

DisplayedFillStrokeSettingsUpdater::DisplayedFillStrokeSettingsUpdater()
{

}

void DisplayedFillStrokeSettingsUpdater::update()
{
    MainWindow::getInstance()->scheduleDisplayedFillStrokeSettingsUpdate();
}

RectangleBottomRightUpdater::RectangleBottomRightUpdater(Rectangle *target) :
    AnimatorUpdater()
{
    mTarget = target;
}

void RectangleBottomRightUpdater::update()
{
    mTarget->updateRadiusXAndRange();
}

PixmapEffectUpdater::PixmapEffectUpdater(BoundingBox *target)
{
    mTarget = target;
}

void PixmapEffectUpdater::update()
{
    mTarget->scheduleEffectsMarginUpdate();
}
