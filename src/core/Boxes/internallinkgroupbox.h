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

#ifndef INTERNALLINKGROUPBOX_H
#define INTERNALLINKGROUPBOX_H
#include "containerbox.h"
#include "Properties/boxtargetproperty.h"

class InternalLinkGroupBox : public ContainerBox {
    e_OBJECT
protected:
    InternalLinkGroupBox(ContainerBox * const linkTarget);
public:
    ~InternalLinkGroupBox()
    { setLinkTarget(nullptr); }

    bool SWT_isLinkBox() const { return true; }
    bool SWT_isGroupBox() const;

    void SWT_setupAbstraction(SWT_Abstraction* abs,
                              const UpdateFuncs &funcs,
                              const int widgetId)
    { BoundingBox::SWT_setupAbstraction(abs, funcs, widgetId); }


    bool SWT_dropSupport(const QMimeData * const data)
    { return BoundingBox::SWT_dropSupport(data); }

    bool SWT_dropIntoSupport(const int index, const QMimeData * const data)
    { return BoundingBox::SWT_dropIntoSupport(index, data); }

    bool SWT_drop(const QMimeData * const data)
    { return BoundingBox::SWT_drop(data); }

    bool SWT_dropInto(const int index, const QMimeData * const data)
    { return BoundingBox::SWT_dropInto(index, data); }

    int prp_getRelFrameShift() const;
    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    FrameRange prp_relInfluenceRange() const;

    void writeBoundingBox(eWriteStream& dst) const
    { BoundingBox::writeBoundingBox(dst); }

    void readBoundingBox(eReadStream& src)
    { BoundingBox::readBoundingBox(src); }

    //bool relPointInsidePath(const QPointF &relPos);
    QPointF getRelCenterPosition();

    bool isFrameInDurationRect(const int relFrame) const;

    stdsptr<BoxRenderData> createRenderData();
    bool isFrameFInDurationRect(const qreal relFrame) const;

    QMatrix getRelativeTransformAtFrame(const qreal relFrame);
    QMatrix getTotalTransformAtFrame(const qreal relFrame);

    SkBlendMode getBlendMode() const;

    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data,
                         Canvas * const scene);

    bool relPointInsidePath(const QPointF &relPos) const;

    void setLinkTarget(ContainerBox * const linkTarget);
    ContainerBox *getLinkTarget() const;
    ContainerBox *getFinalTarget() const;
protected:
    bool isParentLink() const;

    qsptr<BoxTargetProperty> mBoxTarget =
            enve::make_shared<BoxTargetProperty>("link target");
    ConnContextQPtr<ContainerBox> mLinkTarget;
};

#endif // INTERNALLINKGROUPBOX_H
