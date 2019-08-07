#ifndef INTERNALLINKCANVAS_H
#define INTERNALLINKCANVAS_H
#include "internallinkgroupbox.h"
#include "Properties/boolproperty.h"

class InternalLinkCanvas : public InternalLinkGroupBox {
    e_OBJECT
protected:
    InternalLinkCanvas(ContainerBox * const linkTarget);
public:
    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data);

    qsptr<BoundingBox> createLinkForLinkGroup();

    stdsptr<BoxRenderData> createRenderData();

    bool relPointInsidePath(const QPointF &relPos) const;
    void anim_setAbsFrame(const int frame);

    bool clipToCanvas();
private:
    qsptr<BoolProperty> mClipToCanvas =
            enve::make_shared<BoolProperty>("clip");
};

#endif // INTERNALLINKCANVAS_H
