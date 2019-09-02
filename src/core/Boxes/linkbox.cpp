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
#include <QFileDialog>
#include "canvas.h"
#include "Timeline/durationrectangle.h"
#include "Animators/transformanimator.h"
#include "skia/skiahelpers.h"

ExternalLinkBox::ExternalLinkBox() : ContainerBox(TYPE_LAYER) {
    mType = TYPE_EXTERNAL_LINK;
    prp_setName("Link Empty");
}

void ExternalLinkBox::reload() {


    planUpdate(UpdateReason::userChange);
}

void ExternalLinkBox::changeSrc(QWidget* dialogParent) {
    QString src = QFileDialog::getOpenFileName(dialogParent,
                                               "Link File",
                                               "",
                                               "enve Files (*.ev)");
    if(!src.isEmpty()) setSrc(src);
}

void ExternalLinkBox::setSrc(const QString &src) {
    mSrc = src;
    prp_setName("Link " + src);
    reload();
}

InternalLinkBox::InternalLinkBox(BoundingBox * const linkTarget) :
    BoundingBox(TYPE_INTERNAL_LINK) {
    setLinkTarget(linkTarget);
    ca_prependChildAnimator(mTransformAnimator.data(), mBoxTarget);
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

qsptr<BoundingBox> InternalLinkBox::createLinkForLinkGroup() {
    if(isParentLink()) {
        Q_ASSERT(getLinkTarget());
        return getLinkTarget()->createLinkForLinkGroup();
    } else return BoundingBox::createLink();
}

stdsptr<BoxRenderData> InternalLinkBox::createRenderData() {
    const auto renderData = getLinkTarget()->createRenderData();
    renderData->fParentBox = this;
    return renderData;
}

void InternalLinkBox::setupRenderData(
        const qreal relFrame, BoxRenderData * const data) {
    const auto linkTarget = getLinkTarget();
    if(linkTarget) linkTarget->setupRenderData(relFrame, data);

    BoundingBox::setupRenderData(relFrame, data);
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
    if(mVisible) {
        if(isFrameInDurationRect(relFrame)) {
            range *= BoundingBox::prp_getIdenticalRelRange(relFrame);
        } else {
            if(relFrame > mDurationRectangle->getMaxRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheRight();
            } else if(relFrame < mDurationRectangle->getMinRelFrame()) {
                range = mDurationRectangle->getRelFrameRangeToTheLeft();
            }
        }
    }
    auto targetRange = getLinkTarget()->prp_getIdenticalRelRange(relFrame);

    return range*targetRange;
}
