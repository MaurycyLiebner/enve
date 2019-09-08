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

#include "boxwithpatheffects.h"
#include "../PathEffects/patheffectanimators.h"
#include "../PathEffects/patheffectstask.h"

BoxWithPathEffects::BoxWithPathEffects(const eBoxType type) :
    BoundingBox(type) {
    mPathEffectsAnimators = enve::make_shared<PathEffectAnimators>();
    mPathEffectsAnimators->prp_setName("path effects");

    mFillPathEffectsAnimators = enve::make_shared<PathEffectAnimators>();
    mFillPathEffectsAnimators->prp_setName("fill effects");

    mOutlineBasePathEffectsAnimators = enve::make_shared<PathEffectAnimators>();
    mOutlineBasePathEffectsAnimators->prp_setName("outline base effects");

    mOutlinePathEffectsAnimators = enve::make_shared<PathEffectAnimators>();
    mOutlinePathEffectsAnimators->prp_setName("outline effects");

    ca_addChild(mPathEffectsAnimators);
    ca_addChild(mFillPathEffectsAnimators);
    ca_addChild(mOutlineBasePathEffectsAnimators);
    ca_addChild(mOutlinePathEffectsAnimators);
}

void BoxWithPathEffects::setupTreeViewMenu(PropertyMenu * const menu) {
    const PropertyMenu::CheckSelectedOp<BoxWithPathEffects> pathOp =
    [](BoxWithPathEffects* const box, const bool checked) {
        box->setPathEffectsEnabled(checked);
    };
    menu->addCheckableAction("Path Effects",
                             getPathEffectsVisible(), pathOp)->
            setDisabled(mPathEffectsAnimators->hasChildAnimators());

    const PropertyMenu::CheckSelectedOp<BoxWithPathEffects> fillOp =
    [](BoxWithPathEffects* const box, const bool checked) {
        box->setFillEffectsEnabled(checked);
    };
    menu->addCheckableAction("Fill Effects",
                             getFillEffectsVisible(), fillOp)->
            setDisabled(mFillPathEffectsAnimators->hasChildAnimators());

    const PropertyMenu::CheckSelectedOp<BoxWithPathEffects> outBaseOp =
    [](BoxWithPathEffects* const box, const bool checked) {
        box->setOutlineBaseEffectsEnabled(checked);
    };
    menu->addCheckableAction("Outline Base Effects",
                             getOutlineBaseEffectsVisible(), outBaseOp)->
            setDisabled(mOutlineBasePathEffectsAnimators->hasChildAnimators());

    const PropertyMenu::CheckSelectedOp<BoxWithPathEffects> outOp =
    [](BoxWithPathEffects* const box, const bool checked) {
        box->setOutlineEffectsEnabled(checked);
    };
    menu->addCheckableAction("Outline Effects",
                             getOutlineEffectsVisible(), outOp)->
            setDisabled(mOutlinePathEffectsAnimators->hasChildAnimators());

    menu->addSeparator();

    BoundingBox::setupTreeViewMenu(menu);
}

void BoxWithPathEffects::addPathEffect(const qsptr<PathEffect>& effect) {
    mPathEffectsAnimators->addChild(effect);
}

void BoxWithPathEffects::addFillPathEffect(const qsptr<PathEffect>& effect) {
    mFillPathEffectsAnimators->addChild(effect);
}

void BoxWithPathEffects::addOutlineBasePathEffect(const qsptr<PathEffect>& effect) {
    mOutlineBasePathEffectsAnimators->addChild(effect);
}

void BoxWithPathEffects::addOutlinePathEffect(const qsptr<PathEffect>& effect) {
    mOutlinePathEffectsAnimators->addChild(effect);
}

void BoxWithPathEffects::setPathEffectsEnabled(const bool enable) {
    mPathEffectsAnimators->SWT_setVisible(
                mPathEffectsAnimators->hasChildAnimators() || enable);
}

bool BoxWithPathEffects::getPathEffectsVisible() const {
    return mPathEffectsAnimators->SWT_isVisible();
}

void BoxWithPathEffects::setFillEffectsEnabled(const bool enable) {
    mFillPathEffectsAnimators->SWT_setVisible(
                mFillPathEffectsAnimators->hasChildAnimators() || enable);
}

bool BoxWithPathEffects::getFillEffectsVisible() const {
    return mFillPathEffectsAnimators->SWT_isVisible();
}

void BoxWithPathEffects::setOutlineBaseEffectsEnabled(const bool enable) {
    mOutlinePathEffectsAnimators->SWT_setVisible(
                mOutlineBasePathEffectsAnimators->hasChildAnimators() || enable);
}

bool BoxWithPathEffects::getOutlineBaseEffectsVisible() const {
    return mOutlineBasePathEffectsAnimators->SWT_isVisible();
}

void BoxWithPathEffects::setOutlineEffectsEnabled(const bool enable) {
    mOutlinePathEffectsAnimators->SWT_setVisible(
                mOutlinePathEffectsAnimators->hasChildAnimators() || enable);
}

bool BoxWithPathEffects::getOutlineEffectsVisible() const {
    return mOutlinePathEffectsAnimators->SWT_isVisible();
}

PathEffectAnimators *BoxWithPathEffects::getPathEffectsAnimators() {
    return mPathEffectsAnimators.data();
}

PathEffectAnimators *BoxWithPathEffects::getFillPathEffectsAnimators() {
    return mFillPathEffectsAnimators.data();
}

PathEffectAnimators *BoxWithPathEffects::getOutlineBasrPathEffectsAnimators() {
    return mOutlineBasePathEffectsAnimators.data();
}

PathEffectAnimators *BoxWithPathEffects::getOutlinePathEffectsAnimators() {
    return mOutlinePathEffectsAnimators.data();
}

bool BoxWithPathEffects::differenceInPathBetweenFrames(const int frame1, const int frame2) const {
    if(mPathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(!mParentGroup) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);
    return mParentGroup->differenceInPathBetweenFrames(pFrame1, pFrame2);
}

bool BoxWithPathEffects::differenceInOutlinePathBetweenFrames(const int frame1, const int frame2) const {
    if(mOutlineBasePathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(mOutlinePathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(!mParentGroup) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);
    return mParentGroup->differenceInOutlinePathBetweenFrames(pFrame1, pFrame2);
}

bool BoxWithPathEffects::differenceInFillPathBetweenFrames(const int frame1, const int frame2) const {
    if(mFillPathEffectsAnimators->prp_differencesBetweenRelFrames(frame1, frame2))
        return true;
    if(!mParentGroup) return false;
    const int absFrame1 = prp_relFrameToAbsFrame(frame1);
    const int absFrame2 = prp_relFrameToAbsFrame(frame2);
    const int pFrame1 = mParentGroup->prp_absFrameToRelFrame(absFrame1);
    const int pFrame2 = mParentGroup->prp_absFrameToRelFrame(absFrame2);
    return mParentGroup->differenceInFillPathBetweenFrames(pFrame1, pFrame2);
}

void BoxWithPathEffects::addPathEffects(const qreal relFrame,
                                        QList<stdsptr<PathEffectCaller>>& list) {
    mPathEffectsAnimators->addEffects(relFrame, list);
    if(!mParentGroup) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame =
            mParentGroup->prp_absFrameToRelFrameF(absFrame);
    mParentGroup->addPathEffects(parentRelFrame, list);
}

void BoxWithPathEffects::addFillEffects(const qreal relFrame,
                                        QList<stdsptr<PathEffectCaller>>& list) {
    mFillPathEffectsAnimators->addEffects(relFrame, list);
    if(!mParentGroup) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame =
            mParentGroup->prp_absFrameToRelFrameF(absFrame);
    mParentGroup->addFillEffects(parentRelFrame, list);
}

void BoxWithPathEffects::addOutlineBaseEffects(const qreal relFrame,
                                               QList<stdsptr<PathEffectCaller>>& list) {
    mOutlineBasePathEffectsAnimators->addEffects(relFrame, list);
    if(!mParentGroup) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame =
            mParentGroup->prp_absFrameToRelFrameF(absFrame);
    mParentGroup->addOutlineBaseEffects(parentRelFrame, list);
}

void BoxWithPathEffects::addOutlineEffects(const qreal relFrame,
                                           QList<stdsptr<PathEffectCaller>>& list) {
    mOutlinePathEffectsAnimators->addEffects(relFrame, list);
    if(!mParentGroup) return;
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal parentRelFrame =
            mParentGroup->prp_absFrameToRelFrameF(absFrame);
    mParentGroup->addOutlineEffects(parentRelFrame, list);
}

void BoxWithPathEffects::getMotionBlurProperties(QList<Property*> &list) const {
    BoundingBox::getMotionBlurProperties(list);
    list.append(mPathEffectsAnimators.get());
    list.append(mFillPathEffectsAnimators.get());
    list.append(mOutlinePathEffectsAnimators.get());
}
