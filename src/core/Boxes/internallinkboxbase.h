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

#ifndef INTERNALLINKBOXBASE_H
#define INTERNALLINKBOXBASE_H

#include "boundingbox.h"
#include "Timeline/durationrectangle.h"

template <typename BoxT>
class InternalLinkBoxBase : public BoxT {
protected:
    using BoxT::BoxT;
public:
    bool SWT_isLinkBox() const override { return true; }

    FrameRange prp_getIdenticalRelRange(const int relFrame) const override;
    FrameRange prp_relInfluenceRange() const override;
    int prp_getRelFrameShift() const override;

    void writeBoundingBox(eWriteStream& dst) const override
    { BoundingBox::writeBoundingBox(dst); }

    void readBoundingBox(eReadStream& src) override
    { BoundingBox::readBoundingBox(src); }

    bool relPointInsidePath(const QPointF &relPos) const override;
    QPointF getRelCenterPosition() override;

    stdsptr<BoxRenderData> createRenderData() override;

    SkBlendMode getBlendMode() const override;

    QMatrix getRelativeTransformAtFrame(const qreal relFrame) override;
    QMatrix getTotalTransformAtFrame(const qreal relFrame) override;

    bool isFrameInDurationRect(const int relFrame) const override;
    bool isFrameFInDurationRect(const qreal relFrame) const override;

    HardwareSupport hardwareSupport() const override;
protected:
    ConnContext& assignLinkTarget(BoxT * const linkTarget)
    { return mLinkTarget.assign(linkTarget); }
    BoxT *getLinkTarget() const
    { return mLinkTarget; }
    BoxT *getFinalTarget() const;

    bool isParentLink() const;

    qsptr<BoxTargetProperty> mBoxTarget =
            enve::make_shared<BoxTargetProperty>("link target");
private:
    ConnContextQPtr<BoxT> mLinkTarget;
};

template <typename BoxT>
bool InternalLinkBoxBase<BoxT>::isParentLink() const {
    const auto parentGroup = this->getParentGroup();
    if(!parentGroup) return false;
    return parentGroup->SWT_isLinkBox();
}

template <typename BoxT>
bool InternalLinkBoxBase<BoxT>::relPointInsidePath(const QPointF &relPos) const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return false;
    return linkTarget->relPointInsidePath(relPos);
}

template <typename BoxT>
QPointF InternalLinkBoxBase<BoxT>::getRelCenterPosition() {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return QPointF();
    return linkTarget->getRelCenterPosition();
}

template <typename BoxT>
SkBlendMode InternalLinkBoxBase<BoxT>::getBlendMode() const {
    const auto linkTarget = getLinkTarget();
    if(isParentLink() && linkTarget) {
        return linkTarget->getBlendMode();
    }
    return BoxT::getBlendMode();
}

template <typename BoxT>
HardwareSupport InternalLinkBoxBase<BoxT>::hardwareSupport() const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return BoxT::hardwareSupport();
    return linkTarget->hardwareSupport();
}

template <typename BoxT>
bool InternalLinkBoxBase<BoxT>::isFrameInDurationRect(const int relFrame) const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return false;
    return BoxT::isFrameInDurationRect(relFrame) &&
           linkTarget->isFrameInDurationRect(relFrame);
}

template <typename BoxT>
bool InternalLinkBoxBase<BoxT>::isFrameFInDurationRect(const qreal relFrame) const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return false;
    return BoxT::isFrameFInDurationRect(relFrame) &&
           linkTarget->isFrameFInDurationRect(relFrame);
}

template <typename BoxT>
QMatrix InternalLinkBoxBase<BoxT>::getRelativeTransformAtFrame(const qreal relFrame) {
    if(isParentLink()) {
        const auto linkTarget = getLinkTarget();
        if(!linkTarget) return QMatrix();
        return linkTarget->getRelativeTransformAtFrame(relFrame);
    } else {
        return BoundingBox::getRelativeTransformAtFrame(relFrame);
    }
}

template <typename BoxT>
QMatrix InternalLinkBoxBase<BoxT>::getTotalTransformAtFrame(const qreal relFrame) {
    if(isParentLink()) {
        const auto parentGroup = this->getParentGroup();
        const auto linkTarget = getLinkTarget();
        if(!linkTarget || !parentGroup) return QMatrix();
        return linkTarget->getRelativeTransformAtFrame(relFrame)*
               parentGroup->getTotalTransformAtFrame(relFrame);
    } else {
        return BoundingBox::getTotalTransformAtFrame(relFrame);
    }
}

template <typename BoxT>
BoxT *InternalLinkBoxBase<BoxT>::getFinalTarget() const {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return nullptr;
    if(linkTarget->SWT_isLinkBox()) {
        const auto targetAsLink = static_cast<InternalLinkBoxBase<BoxT>*>(linkTarget);
        return targetAsLink->getFinalTarget();
    }
    return linkTarget;
}

template <typename BoxT>
FrameRange InternalLinkBoxBase<BoxT>::prp_getIdenticalRelRange(const int relFrame) const {
    FrameRange range{FrameRange::EMIN, FrameRange::EMAX};
    const auto linkTarget = getLinkTarget();
    if(this->isVisible() && linkTarget)
        range *= BoxT::prp_getIdenticalRelRange(relFrame);
    else return range;
    auto targetRange = linkTarget->prp_getIdenticalRelRange(relFrame);
    return range*targetRange;
}

template <typename BoxT>
FrameRange InternalLinkBoxBase<BoxT>::prp_relInfluenceRange() const {
    const auto linkTarget = getLinkTarget();
    FrameRange inflRange;
    const auto durRect = this->getDurationRectangle();
    if(durRect) inflRange = durRect->getRelFrameRange();
    else inflRange = ComplexAnimator::prp_relInfluenceRange();
    if(linkTarget) {
        return inflRange*linkTarget->prp_relInfluenceRange();
    } else return inflRange;
}

template <typename BoxT>
int InternalLinkBoxBase<BoxT>::prp_getRelFrameShift() const {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) {
        return linkTarget->prp_getRelFrameShift() +
               BoxT::prp_getRelFrameShift();
    } else return BoxT::prp_getRelFrameShift();
}

template <typename BoxT>
stdsptr<BoxRenderData> InternalLinkBoxBase<BoxT>::createRenderData() {
    const auto linkTarget = getLinkTarget();
    if(!linkTarget) return nullptr;
    const auto renderData = linkTarget->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

#endif // INTERNALLINKBOXBASE_H
