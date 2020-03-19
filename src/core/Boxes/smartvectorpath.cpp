// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "smartvectorpath.h"
#include <QPainter>
#include "canvas.h"
#include <QDebug>
#include "undoredo.h"
#include "MovablePoints/pathpivot.h"
#include "pointhelpers.h"
#include "Animators/SmartPath/smartpathcollection.h"
#include "Animators/gradientpoints.h"
#include "Animators/transformanimator.h"
#include "MovablePoints/segment.h"
#include "Animators/SmartPath/smartpathanimator.h"
#include "PathEffects/patheffectcollection.h"
#include "Animators/qpointfanimator.h"

SmartVectorPath::SmartVectorPath() :
    PathBox("Path", eBoxType::vectorPath) {
    mPathAnimator = enve::make_shared<SmartPathCollection>();
    connect(mPathAnimator.get(), &Property::prp_currentFrameChanged,
            this, [this](const UpdateReason reason) {
        setPathsOutdated(reason);
    });
    ca_prependChild(mPathEffectsAnimators.data(), mPathAnimator);
}

bool SmartVectorPath::differenceInEditPathBetweenFrames(
        const int frame1, const int frame2) const {
    return mPathAnimator->prp_differencesBetweenRelFrames(frame1, frame2);
}

QDomElement SmartVectorPath::saveSVG(QDomDocument& doc,
                                     QDomElement& defs,
                                     const FrameRange& absRange,
                                     const qreal fps) const {
    auto ele = doc.createElement("path");
    mPathAnimator->savePathsSVG(doc, ele, defs, absRange, fps);
    savePathBoxSVG(doc, ele, defs, absRange, fps);
    return ele;
}

void SmartVectorPath::loadSkPath(const SkPath &path) {
    mPathAnimator->loadSkPath(path);
}

SmartPathCollection *SmartVectorPath::getPathAnimator() {
    return mPathAnimator.data();
}

#include "typemenu.h"
void SmartVectorPath::setupCanvasMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<SmartVectorPath>()) return;
    menu->addedActionsForType<SmartVectorPath>();
    PathBox::setupCanvasMenu(menu);
    PropertyMenu::PlainSelectedOp<SmartVectorPath> op = [](SmartVectorPath * box) {
        box->applyCurrentTransform();
    };
    menu->addSeparator();
    menu->addPlainAction("Apply Transformation", op);
}

void SmartVectorPath::applyCurrentTransform() {
    prp_pushUndoRedoName("Apply Transform");
    mNReasonsNotToApplyUglyTransform++;
    const auto transform = mTransformAnimator->getRotScaleShearTransform();
    mPathAnimator->applyTransform(transform);
    getFillSettings()->applyTransform(transform);
    getStrokeSettings()->applyTransform(transform);
    mTransformAnimator->startRotScaleShearTransform();
    mTransformAnimator->resetRotScaleShear();
    mTransformAnimator->prp_finishTransform();
    mNReasonsNotToApplyUglyTransform--;
}

SkPath SmartVectorPath::getRelativePath(const qreal relFrame) const {
     return mPathAnimator->getPathAtRelFrame(relFrame);
}

void SmartVectorPath::getMotionBlurProperties(QList<Property*> &list) const {
    PathBox::getMotionBlurProperties(list);
    list.append(mPathAnimator.get());
}

QList<qsptr<SmartVectorPath>> SmartVectorPath::breakPathsApart_k() {
    QList<qsptr<SmartVectorPath>> result;
    const int iMax = mPathAnimator->ca_getNumberOfChildren() - 1;
    if(iMax < 1) return result;
    for(int i = iMax; i >= 0; i--) {
        const auto srcPath = mPathAnimator->takeChildAt(i);
        const auto newPath = enve::make_shared<SmartVectorPath>();
        copyPathBoxDataTo(newPath.get());
        newPath->getPathAnimator()->addChild(srcPath);
        result.append(newPath);
    }
    removeFromParent_k();
    return result;
}
