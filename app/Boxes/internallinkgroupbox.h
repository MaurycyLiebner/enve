#ifndef INTERNALLINKGROUPBOX_H
#define INTERNALLINKGROUPBOX_H
#include "containerbox.h"
#include "Properties/boxtargetproperty.h"

class InternalLinkGroupBox : public ContainerBox {
    Q_OBJECT
    friend class SelfRef;
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

    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);
    //bool relPointInsidePath(const QPointF &relPos);
    QPointF getRelCenterPosition();

    qsptr<BoundingBox> createLink();
    qsptr<BoundingBox> createLinkForLinkGroup();

    bool isFrameInDurationRect(const int &relFrame) const;

    stdsptr<BoundingBoxRenderData> createRenderData();
    QRectF getRelBoundingRect(const qreal &relFrame);
    FrameRange prp_getIdenticalRelRange(const int &relFrame) const;

    QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);

    void setupEffectsF(const qreal &relFrame,
                       BoundingBoxRenderData * const data);

    qreal getEffectsMarginAtRelFrameF(const qreal &relFrame);

    const SkBlendMode &getBlendMode();

    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData * const data);

    int prp_getRelFrameShift() const;

    bool relPointInsidePath(const QPointF &relPos) const;

    void setLinkTarget(ContainerBox * const linkTarget);
    ContainerBox *getLinkTarget() const;
    ContainerBox *getFinalTarget() const;
public slots:
    void setTargetSlot(BoundingBox *target);
protected:
    qsptr<BoxTargetProperty> mBoxTarget =
            SPtrCreate(BoxTargetProperty)("link target");
};

#endif // INTERNALLINKGROUPBOX_H
