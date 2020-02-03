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

#include "internallinkgroupbox.h"
#include "Animators/transformanimator.h"
#include "Timeline/durationrectangle.h"
#include "layerboxrenderdata.h"

InternalLinkGroupBox::InternalLinkGroupBox(ContainerBox * const linkTarget) :
    ContainerBox(eBoxType::internalLinkGroup) {
    prp_setName("Link 0");
    setLinkTarget(linkTarget);

    ca_prependChild(mTransformAnimator.data(), mBoxTarget);
    connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
            this, [this](BoundingBox* const target) {
        const auto cbTarget = dynamic_cast<ContainerBox*>(target);
        setLinkTarget(cbTarget);
    });
}

bool InternalLinkGroupBox::SWT_isGroupBox() const {
    const auto finalTarget = getFinalTarget();
    if(!finalTarget) return false;
    return finalTarget->SWT_isGroupBox();
}

SkBlendMode InternalLinkGroupBox::getBlendMode() const {
    if(isParentLink() && getLinkTarget()) {
        return getLinkTarget()->getBlendMode();
    }
    return BoundingBox::getBlendMode();
}

void InternalLinkGroupBox::setupRenderData(const qreal relFrame,
                                           BoxRenderData * const data,
                                           Canvas* const scene) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->BoundingBox::setupRenderData(relFrame, data, scene);
    ContainerBox::setupRenderData(relFrame, data, scene);
}

ContainerBox *InternalLinkGroupBox::getFinalTarget() const {
    if(!getLinkTarget()) return nullptr;
    if(getLinkTarget()->SWT_isLinkBox()) {
        return static_cast<InternalLinkGroupBox*>(getLinkTarget())->getFinalTarget();
    }
    return getLinkTarget();
}

bool InternalLinkGroupBox::isParentLink() const {
    if(!mParentGroup) return false;
    return mParentGroup->SWT_isLinkBox();
}

bool InternalLinkGroupBox::relPointInsidePath(const QPointF &relPos) const {
    if(!getLinkTarget()) return false;
    return getLinkTarget()->relPointInsidePath(relPos);
}

HardwareSupport InternalLinkGroupBox::hardwareSupport() const {
    if(!getLinkTarget()) return BoundingBox::hardwareSupport();
    return getLinkTarget()->hardwareSupport();
}

bool InternalLinkGroupBox::isFrameInDurationRect(const int relFrame) const {
    if(!getLinkTarget()) return false;
    return ContainerBox::isFrameInDurationRect(relFrame) &&
            getLinkTarget()->isFrameInDurationRect(relFrame);
}

bool InternalLinkGroupBox::isFrameFInDurationRect(const qreal relFrame) const {
    if(!getLinkTarget()) return false;
    return ContainerBox::isFrameFInDurationRect(relFrame) &&
            getLinkTarget()->isFrameFInDurationRect(relFrame);
}

QMatrix InternalLinkGroupBox::getRelativeTransformAtFrame(const qreal relFrame) {
    if(isParentLink()) {
        return getLinkTarget()->getRelativeTransformAtFrame(relFrame);
    } else {
        return BoundingBox::getRelativeTransformAtFrame(relFrame);
    }
}

QMatrix InternalLinkGroupBox::getTotalTransformAtFrame(const qreal relFrame) {
    if(isParentLink()) {
        const auto linkTarget = getLinkTarget();
        return linkTarget->getRelativeTransformAtFrame(relFrame)*
                mParentGroup->getTotalTransformAtFrame(relFrame);
    } else {
        return BoundingBox::getTotalTransformAtFrame(relFrame);
    }
}

FrameRange InternalLinkGroupBox::prp_getIdenticalRelRange(const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    if(mVisible && getLinkTarget())
        range *= ContainerBox::prp_getIdenticalRelRange(relFrame);
    else return range;
    auto targetRange = getLinkTarget()->prp_getIdenticalRelRange(relFrame);
    return range*targetRange;
}

FrameRange InternalLinkGroupBox::prp_relInfluenceRange() const {
    FrameRange inflRange;
    if(mDurationRectangle) inflRange = mDurationRectangle->getRelFrameRange();
    else inflRange = ComplexAnimator::prp_relInfluenceRange();
    if(getLinkTarget()) {
        return inflRange*getLinkTarget()->prp_relInfluenceRange();
    } else return inflRange;
}

int InternalLinkGroupBox::prp_getRelFrameShift() const {
    if(getLinkTarget()) {
        return getLinkTarget()->prp_getRelFrameShift() +
                ContainerBox::prp_getRelFrameShift();
    } else return ContainerBox::prp_getRelFrameShift();
}

#include "Sound/singlesound.h"
#include "Sound/esoundlink.h"

void InternalLinkGroupBox::setLinkTarget(ContainerBox * const linkTarget) {
    if(getLinkTarget()) getLinkTarget()->removeLinkingBox(this);
    removeAllContained();
    mBoxTarget->setTarget(linkTarget);
    auto& conn = mLinkTarget.assign(linkTarget);
    if(linkTarget) {
        rename(linkTarget->prp_getName() + " Link 0");
        conn << connect(linkTarget, &Property::prp_nameChanged,
                       this, &Property::prp_setName);
        linkTarget->addLinkingBox(this);
        conn << connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                this, [this, linkTarget](const FrameRange& targetAbs) {
            const auto relRange = linkTarget->prp_absRangeToRelRange(targetAbs);
            prp_afterChangedRelRange(relRange);
        });

        conn << connect(linkTarget->getTransformAnimator(), &Property::prp_absFrameRangeChanged,
                this, [this, linkTarget](const FrameRange& targetAbs) {
            const auto relRange = linkTarget->prp_absRangeToRelRange(targetAbs);
            mTransformAnimator->prp_afterChangedRelRange(relRange);
        });

        const auto &boxesList = linkTarget->getContained();
        for(int i = boxesList.count() - 1; i >= 0; i--) {
            const auto& child = boxesList.at(i);
            if(child->SWT_isBoundingBox()) {
                const auto box = static_cast<BoundingBox*>(child.get());
                const auto newLink = box->createLink();
                addContained(newLink);
            } else /*(child->SWT_isSound())*/ {
                const auto sound = static_cast<SingleSound*>(child.get());
                const auto newLink = sound->createLink();
                addContained(newLink);
            }
        }
    } else {
        rename("Empty Link 0");
    }
    planUpdate(UpdateReason::userChange);
}

QPointF InternalLinkGroupBox::getRelCenterPosition() {
    if(!getLinkTarget()) return QPointF();
    return getLinkTarget()->getRelCenterPosition();
}

ContainerBox *InternalLinkGroupBox::getLinkTarget() const {
    return mLinkTarget;
}

stdsptr<BoxRenderData> InternalLinkGroupBox::createRenderData() {
    if(!getLinkTarget()) return nullptr;
    auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}
