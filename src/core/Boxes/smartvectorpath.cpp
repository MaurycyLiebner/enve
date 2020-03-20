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
                                     const qreal fps, const bool loop) const {
    const bool baseEffects = hasBasePathEffects();
    const bool outlineBaseEffects = hasOutlineBaseEffects();
    const bool outlineEffects = hasOutlineEffects();
    const bool fillEffects = hasFillEffects();
    const bool splitFillStroke = fillEffects ||
                                 outlineBaseEffects ||
                                 outlineEffects;
    QDomElement result;
    if(splitFillStroke) {
        result = doc.createElement("g");

        auto fill = doc.createElement("path");
        SmartPathCollection::EffectApplier fillApplier;
        if(baseEffects || fillEffects) {
            fillApplier = [this](const int relFrame, SkPath& path) {
                applyBasePathEffects(relFrame, path);
                applyFillEffects(relFrame, path);
            };
        };
        mPathAnimator->savePathsSVG(doc, fill, defs, absRange,
                                    fps, loop, fillApplier,
                                    baseEffects || fillEffects);
        saveFillSettingsSVG(doc, fill, defs, absRange, fps, loop);
        fill.setAttribute("stroke", "none");
        result.appendChild(fill);
        switch(mPathAnimator->getFillType()) {
        case SkPathFillType::kEvenOdd:
            fill.setAttribute("fill-rule", "evenodd");
            break;
        default:
            fill.setAttribute("fill-rule", "nonzero");
        }

        auto stroke = doc.createElement("path");
        SmartPathCollection::EffectApplier strokeApplier;
        if(baseEffects || outlineBaseEffects || outlineEffects) {
            strokeApplier = [this, outlineEffects](const int relFrame, SkPath& path) {
                applyBasePathEffects(relFrame, path);
                applyOutlineBaseEffects(relFrame, path);
                if(!outlineEffects) return;
                const auto strokeSettings = getStrokeSettings();
                SkStroke stroker;
                strokeSettings->setStrokerSettingsForRelFrameSk(relFrame, &stroker);
                stroker.strokePath(path, &path);
                applyOutlineEffects(relFrame, path);
            };
        };
        mPathAnimator->savePathsSVG(doc, stroke, defs, absRange,
                                    fps, loop, strokeApplier,
                                    baseEffects || outlineBaseEffects ||
                                    outlineEffects);
        saveStrokeSettingsSVG(doc, stroke, defs, absRange,
                              fps, loop, outlineEffects);
        stroke.setAttribute(outlineEffects ? "stroke" : "fill", "none");
        if(outlineEffects) stroke.setAttribute("fill-rule", "nonzero");

        result.appendChild(stroke);
    } else {
        result = doc.createElement("path");
        SmartPathCollection::EffectApplier applier;
        if(baseEffects) {
            applier = [this](const int relFrame, SkPath& path) {
                applyBasePathEffects(relFrame, path);
            };
        };
        mPathAnimator->savePathsSVG(doc, result, defs, absRange,
                                    fps, loop, applier, baseEffects);
        savePathBoxSVG(doc, result, defs, absRange, fps, loop);
    }
    return result;
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
