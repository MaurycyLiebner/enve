#ifndef INTERNALLINKCANVAS_H
#define INTERNALLINKCANVAS_H
#include "internallinkgroupbox.h"
#include "Properties/boolproperty.h"

class InternalLinkCanvas : public InternalLinkGroupBox {
    friend class SelfRef;
protected:
    InternalLinkCanvas(ContainerBox * const linkTarget);
public:
    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    void setupRenderData(const qreal relFrame,
                         BoundingBoxRenderData * const data);

    qsptr<BoundingBox> createLinkForLinkGroup();

    stdsptr<BoundingBoxRenderData> createRenderData();

    bool relPointInsidePath(const QPointF &relPos) const;

    bool clipToCanvas();
private:
    qsptr<BoolProperty> mClipToCanvas =
            SPtrCreate(BoolProperty)("clip");
};

#endif // INTERNALLINKCANVAS_H
