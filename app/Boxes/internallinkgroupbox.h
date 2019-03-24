#ifndef INTERNALLINKGROUPBOX_H
#define INTERNALLINKGROUPBOX_H
#include "boxesgroup.h"
#include "Properties/boxtargetproperty.h"

class InternalLinkGroupBox : public BoxesGroup {
    Q_OBJECT
    friend class SelfRef;
protected:
    InternalLinkGroupBox(BoxesGroup * const linkTarget);
public:
    ~InternalLinkGroupBox();
    bool SWT_isLinkBox() const;

    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    //bool relPointInsidePath(const QPointF &relPos);
    QPointF getRelCenterPosition();

    qsptr<BoundingBox> createLink();
    qsptr<BoundingBox> createLinkForLinkGroup();

    bool isRelFrameInVisibleDurationRect(const int &relFrame) const;

    stdsptr<BoundingBoxRenderData> createRenderData();
    QRectF getRelBoundingRectAtRelFrame(const qreal &relFrame);
    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;

    bool SWT_isBoxesGroup() const;

    QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);

    void setupEffectsF(const qreal &relFrame,
                       BoundingBoxRenderData* data);

    qreal getEffectsMarginAtRelFrameF(const qreal &relFrame);

    const SkBlendMode &getBlendMode();

    void setupBoundingBoxRenderDataForRelFrameF(
                            const qreal &relFrame,
                            BoundingBoxRenderData* data);

    int prp_getRelFrameShift() const;

    bool relPointInsidePath(const QPointF &relPos) const;

    void setLinkTarget(BoxesGroup *linkTarget);
    BoxesGroup *getLinkTarget() const;
    BoxesGroup *getFinalTarget() const;
public slots:
    void setTargetSlot(BoundingBox *target);
protected:
    qsptr<BoxTargetProperty> mBoxTarget =
            SPtrCreate(BoxTargetProperty)("link target");
};

#endif // INTERNALLINKGROUPBOX_H
