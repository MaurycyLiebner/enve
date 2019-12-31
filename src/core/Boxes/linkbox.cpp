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

#include "linkbox.h"
#include "GUI/edialogs.h"
#include "canvas.h"
#include "Timeline/durationrectangle.h"
#include "Animators/transformanimator.h"
#include "skia/skiahelpers.h"
#include "videobox.h"
#include "Sound/singlesound.h"

ExternalLinkBox::ExternalLinkBox() : ContainerBox(eBoxType::layer) {
    mType = eBoxType::externalLink;
    prp_setName("Link Empty");
}

void ExternalLinkBox::reload() {


    planUpdate(UpdateReason::userChange);
}

void ExternalLinkBox::changeSrc() {
    QString src = eDialogs::openFile("Link File", mSrc,
                                     "enve Files (*.ev)");
    if(!src.isEmpty()) setSrc(src);
}

void ExternalLinkBox::setSrc(const QString &src) {
    mSrc = src;
    prp_setName("Link " + src);
    reload();
}

InternalLinkBox::InternalLinkBox(BoundingBox * const linkTarget) :
    BoundingBox(eBoxType::internalLink) {
    setLinkTarget(linkTarget);
    ca_prependChild(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkBox::setTargetSlot);
}

void InternalLinkBox::setLinkTarget(BoundingBox * const linkTarget) {
    mSound.reset();
    disconnect(mBoxTarget.data(), nullptr, this, nullptr);
    if(getLinkTarget()) {
        disconnect(getLinkTarget(), nullptr, this, nullptr);
        getLinkTarget()->removeLinkingBox(this);
    }
    if(linkTarget) {
        prp_setName(linkTarget->prp_getName() + " link");
        mBoxTarget->setTarget(linkTarget);
        linkTarget->addLinkingBox(this);
        connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                this, [this, linkTarget](const FrameRange& range) {
            const auto relRange = linkTarget->prp_absRangeToRelRange(range);
            prp_afterChangedRelRange(relRange);
        });
        if(linkTarget->SWT_isVideoBox()) {
            const auto vidBox = static_cast<VideoBox*>(linkTarget);
            mSound = vidBox->sound()->createLink();
            connect(this, &eBoxOrSound::parentChanged,
                    mSound.get(), &eBoxOrSound::setParentGroup);
            mSound->setParentGroup(mParentGroup);
        }
    } else {
        prp_setName("empty link");
        mBoxTarget->setTarget(nullptr);
    }
    planUpdate(UpdateReason::userChange);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, &InternalLinkBox::setTargetSlot);
}

QPointF InternalLinkBox::getRelCenterPosition() {
    if(!getLinkTarget()) return QPointF();
    return getLinkTarget()->getRelCenterPosition();
}

BoundingBox *InternalLinkBox::getLinkTarget() const {
    return mBoxTarget->getTarget();
}

stdsptr<BoxRenderData> InternalLinkBox::createRenderData() {
    const auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

void InternalLinkBox::setupRenderData(const qreal relFrame,
                                      BoxRenderData * const data,
                                      Canvas* const scene) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->setupRenderData(relFrame, data, scene);
    BoundingBox::setupRenderData(relFrame, data, scene);
}

bool InternalLinkBox::relPointInsidePath(const QPointF &relPos) const {
    if(!getLinkTarget()) return false;
    return getLinkTarget()->relPointInsidePath(relPos);
}

bool InternalLinkBox::isFrameInDurationRect(const int relFrame) const {
    if(!getLinkTarget()) return false;
    return BoundingBox::isFrameInDurationRect(relFrame) &&
            getLinkTarget()->isFrameInDurationRect(relFrame);
}

bool InternalLinkBox::isFrameFInDurationRect(const qreal relFrame) const {
    if(!getLinkTarget()) return false;
    return BoundingBox::isFrameFInDurationRect(relFrame) &&
            getLinkTarget()->isFrameFInDurationRect(relFrame);
}

FrameRange InternalLinkBox::prp_getIdenticalRelRange(const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible && getLinkTarget())
        range *= BoundingBox::prp_getIdenticalRelRange(relFrame);
    else return range;
    auto targetRange = getLinkTarget()->prp_getIdenticalRelRange(relFrame);
    return range*targetRange;
}

FrameRange InternalLinkBox::prp_relInfluenceRange() const {
    FrameRange inflRange;
    if(mDurationRectangle) inflRange = mDurationRectangle->getRelFrameRange();
    else inflRange = ComplexAnimator::prp_relInfluenceRange();
    if(getLinkTarget()) {
        return inflRange*getLinkTarget()->prp_relInfluenceRange();
    } else return inflRange;
}

int InternalLinkBox::prp_getRelFrameShift() const {
    if(getLinkTarget()) {
        return getLinkTarget()->prp_getRelFrameShift() +
                BoundingBox::prp_getRelFrameShift();
    } else return BoundingBox::prp_getRelFrameShift();
}
