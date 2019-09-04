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

#ifndef INTERNALLINKGROUPBOX_H
#define INTERNALLINKGROUPBOX_H
#include "containerbox.h"
#include "Properties/boxtargetproperty.h"

class InternalLinkGroupBox : public ContainerBox {
    e_OBJECT
protected:
    InternalLinkGroupBox(ContainerBox * const linkTarget);
public:
    ~InternalLinkGroupBox();
    bool SWT_isLinkBox() const { return true; }
    bool SWT_isGroupBox() const {
        const auto finalTarget = getFinalTarget();
        if(!finalTarget) return false;
        return finalTarget->SWT_isGroupBox();
    }

    void SWT_setupAbstraction(SWT_Abstraction* abstraction,
                              const UpdateFuncs &updateFuncs,
                              const int visiblePartWidgetId) {
        BoundingBox::SWT_setupAbstraction(abstraction, updateFuncs,
                                          visiblePartWidgetId);
    }


    bool SWT_dropSupport(const QMimeData * const data) {
        return BoundingBox::SWT_dropSupport(data);
    }

    bool SWT_dropIntoSupport(const int index, const QMimeData * const data) {
        return BoundingBox::SWT_dropIntoSupport(index, data);
    }

    bool SWT_drop(const QMimeData * const data) {
        return BoundingBox::SWT_drop(data);
    }

    bool SWT_dropInto(const int index, const QMimeData * const data) {
        return BoundingBox::SWT_dropInto(index, data);
    }

    void writeBoundingBox(eWriteStream& dst);
    void readBoundingBox(eReadStream& src);
    //bool relPointInsidePath(const QPointF &relPos);
    QPointF getRelCenterPosition();

    bool isFrameInDurationRect(const int relFrame) const;

    stdsptr<BoxRenderData> createRenderData();
    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    FrameRange prp_relInfluenceRange() const;
    bool isFrameFInDurationRect(const qreal relFrame) const;

    QMatrix getRelativeTransformAtFrame(const qreal relFrame) {
        if(isParentLink()) {
            return getLinkTarget()->getRelativeTransformAtFrame(relFrame);
        } else {
            return BoundingBox::getRelativeTransformAtFrame(relFrame);
        }
    }

    QMatrix getTotalTransformAtFrame(const qreal relFrame) {
        if(isParentLink()) {
            const auto linkTarget = getLinkTarget();
            return linkTarget->getRelativeTransformAtFrame(relFrame)*
                    mParentGroup->getTotalTransformAtFrame(relFrame);
        } else {
            return BoundingBox::getTotalTransformAtFrame(relFrame);
        }
    }

    SkBlendMode getBlendMode();

    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data,
                         Canvas * const scene);

    int prp_getRelFrameShift() const;

    bool relPointInsidePath(const QPointF &relPos) const;

    void setTargetSlot(BoundingBox * const target);
    void setLinkTarget(ContainerBox * const linkTarget);
    ContainerBox *getLinkTarget() const;
    ContainerBox *getFinalTarget() const;
protected:
    bool isParentLink() const {
        return mParentGroup ? mParentGroup->SWT_isLinkBox() : false;
    }

    qsptr<BoxTargetProperty> mBoxTarget =
            enve::make_shared<BoxTargetProperty>("link target");
};

#endif // INTERNALLINKGROUPBOX_H
