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

#ifndef LINKBOX_H
#define LINKBOX_H
#include "Boxes/containerbox.h"
#include "Properties/boxtargetproperty.h"
#include "Properties/boolproperty.h"

class eSound;

class ExternalLinkBox : public ContainerBox {
    e_OBJECT
public:
    void reload();

    void changeSrc();

    void setSrc(const QString &src);
private:
    ExternalLinkBox();
    QString mSrc;
};

class InternalLinkBox : public BoundingBox {
    e_OBJECT
protected:
    InternalLinkBox(BoundingBox * const linkTarget);
public:
    ~InternalLinkBox() {
        setLinkTarget(nullptr);
    }

    bool SWT_isLinkBox() const { return true; }

    void writeBoundingBox(eWriteStream& dst) {
        BoundingBox::writeBoundingBox(dst);
    }

    void readBoundingBox(eReadStream& src) {
        BoundingBox::readBoundingBox(src);
    }

    void setLinkTarget(BoundingBox * const linkTarget);

    bool relPointInsidePath(const QPointF &relPos) const;
    QPointF getRelCenterPosition();

    stdsptr<BoxRenderData> createRenderData();
    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data,
                         Canvas * const scene);
    SkBlendMode getBlendMode() {
        if(isParentLink()) return getLinkTarget()->getBlendMode();
        return BoundingBox::getBlendMode();
    }

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    FrameRange prp_relInfluenceRange() const;
    int prp_getRelFrameShift() const;

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

    bool isFrameInDurationRect(const int relFrame) const;
    bool isFrameFInDurationRect(const qreal relFrame) const;
    BoundingBox *getLinkTarget() const;
    void setTargetSlot(BoundingBox * const target) {
        setLinkTarget(target);
    }
protected:
    bool isParentLink() const {
        return mParentGroup ? mParentGroup->SWT_isLinkBox() : false;
    }

    qsptr<BoxTargetProperty> mBoxTarget =
            enve::make_shared<BoxTargetProperty>("link target");
private:
    qsptr<eSound> mSound;
};

#endif // LINKBOX_H
