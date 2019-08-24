// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "Boxes/rectangle.h"
#include "canvas.h"
#include "Animators/gradientpoints.h"
#include "MovablePoints/animatedpoint.h"
#include "Animators/transformanimator.h"
#include "Animators/rastereffectanimators.h"

Rectangle::Rectangle() : PathBox(TYPE_RECTANGLE) {
    prp_setName("Rectangle");

    setPointsHandler(enve::make_shared<PointsHandler>());

    mTopLeftAnimator = enve::make_shared<QPointFAnimator>("top left");
    mTopLeftPoint = enve::make_shared<AnimatedPoint>(
                mTopLeftAnimator.get(), mTransformAnimator.data(),
                TYPE_PATH_POINT);
    mPointsHandler->appendPt(mTopLeftPoint);
    mTopLeftPoint->setRelativePos(QPointF(0, 0));

    mBottomRightAnimator = enve::make_shared<QPointFAnimator>("bottom right");
    mBottomRightPoint = enve::make_shared<AnimatedPoint>(
                mBottomRightAnimator.get(), mTransformAnimator.data(),
                TYPE_PATH_POINT);
    mPointsHandler->appendPt(mBottomRightPoint);
    mBottomRightPoint->setRelativePos(QPointF(10, 10));

    //mTopLeftPoint->setBottomRightPoint(mBottomRightPoint);
    //mBottomRightPoint->setRadiusPoint(mRadiusPoint);

    ca_addChild(mTopLeftAnimator);
    ca_addChild(mBottomRightAnimator);

    ca_prependChildAnimator(mTopLeftAnimator.get(), mRasterEffectsAnimators);
    ca_prependChildAnimator(mBottomRightAnimator.get(), mRasterEffectsAnimators);

    mRadiusAnimator = enve::make_shared<QPointFAnimator>("round radius");
    mRadiusAnimator->setValuesRange(0, 9999);

    mRadiusPoint = enve::make_shared<AnimatedPoint>(
                mRadiusAnimator.get(), mTransformAnimator.data(),
                TYPE_PATH_POINT);

    ca_addChild(mRadiusAnimator);
    ca_prependChildAnimator(mRadiusAnimator.get(), mRasterEffectsAnimators);

    const auto pathUpdater = [this](const UpdateReason reason) {
        setPathsOutdated(reason);
    };
    connect(mTopLeftAnimator.get(), &Property::prp_currentFrameChanged,
            this, pathUpdater);
    connect(mBottomRightAnimator.get(), &Property::prp_currentFrameChanged,
            this, pathUpdater);
    connect(mRadiusAnimator.get(), &Property::prp_currentFrameChanged,
            this, pathUpdater);
}

SkPath Rectangle::getPathAtRelFrameF(const qreal relFrame) {
    SkPath path;
    const SkPoint topLeft =
            toSkPoint(mTopLeftAnimator->
                                getEffectiveValue(relFrame));
    const SkPoint bottomRight =
            toSkPoint(mBottomRightAnimator->
                                getEffectiveValue(relFrame));
    const QPointF radiusAtFrame =
            mRadiusAnimator->getEffectiveValue(relFrame);
    path.addRoundRect(SkRect::MakeLTRB(topLeft.x(), topLeft.y(),
                                       bottomRight.x(), bottomRight.y()),
                      toSkScalar(radiusAtFrame.x()),
                      toSkScalar(radiusAtFrame.y()));
    return path;
}

void Rectangle::setTopLeftPos(const QPointF &pos) {
    mTopLeftPoint->setRelativePos(pos);
}

void Rectangle::setBottomRightPos(const QPointF &pos) {
    mBottomRightPoint->setRelativePos(pos);
}

void Rectangle::setYRadius(const qreal radiusY) {
    mRadiusAnimator->getYAnimator()->setCurrentBaseValue(radiusY);
}

void Rectangle::setXRadius(const qreal radiusX) {
    mRadiusAnimator->getXAnimator()->setCurrentBaseValue(radiusX);
}

void Rectangle::moveSizePointByAbs(const QPointF &absTrans) {
    mBottomRightPoint->moveByAbs(absTrans);
}

MovablePoint *Rectangle::getBottomRightPoint() {
    return mBottomRightPoint.get();
}

void Rectangle::getMotionBlurProperties(QList<Property*> &list) const {
    PathBox::getMotionBlurProperties(list);
    list.append(mTopLeftAnimator.get());
    list.append(mBottomRightAnimator.get());
    list.append(mRadiusAnimator.get());
}

bool Rectangle::differenceInEditPathBetweenFrames(
        const int frame1, const int frame2) const {
    if(mTopLeftAnimator->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    if(mBottomRightAnimator->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return mRadiusAnimator->prp_differencesBetweenRelFrames(frame1, frame2);
}
