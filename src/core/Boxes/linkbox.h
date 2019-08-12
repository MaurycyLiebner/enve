#ifndef LINKBOX_H
#define LINKBOX_H
#include "canvas.h"
#include "Properties/boxtargetproperty.h"
#include "Properties/boolproperty.h"

class ExternalLinkBox : public ContainerBox {
    e_OBJECT
public:
    void reload();

    void changeSrc(QWidget *dialogParent);

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

    void writeBoundingBox(QIODevice * const target) {
        BoundingBox::writeBoundingBox(target);
    }

    void readBoundingBox(QIODevice * const target) {
        BoundingBox::readBoundingBox(target);
    }

    void setLinkTarget(BoundingBox * const linkTarget) {
        disconnect(mBoxTarget.data(), nullptr, this, nullptr);
        if(getLinkTarget()) {
            disconnect(getLinkTarget(), nullptr, this, nullptr);
            getLinkTarget()->removeLinkingBox(this);
        }
        if(!linkTarget) {
            prp_setName("empty link");
            mBoxTarget->setTarget(nullptr);
        } else {
            prp_setName(linkTarget->prp_getName() + " link");
            mBoxTarget->setTarget(linkTarget);
            linkTarget->addLinkingBox(this);
            connect(linkTarget, &BoundingBox::prp_absFrameRangeChanged,
                    this, &BoundingBox::prp_afterChangedAbsRange);
        }
        planScheduleUpdate(UpdateReason::userChange);
        connect(mBoxTarget.data(), &BoxTargetProperty::targetSet,
                this, &InternalLinkBox::setTargetSlot);
    }

    bool relPointInsidePath(const QPointF &relPos) const;
    QPointF getRelCenterPosition();

    qsptr<BoundingBox> createLink();

    qsptr<BoundingBox> createLinkForLinkGroup();

    stdsptr<BoxRenderData> createRenderData();
    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data);
    const SkBlendMode &getBlendMode() {
        if(isParentLink()) return getLinkTarget()->getBlendMode();
        return BoundingBox::getBlendMode();
    }

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;

    QMatrix getRelativeTransformAtFrame(const qreal relFrame) {
        if(mParentGroup ? mParentGroup->SWT_isLinkBox() : false) {
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
};

#endif // LINKBOX_H
