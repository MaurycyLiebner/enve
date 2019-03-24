#ifndef INTERNALLINKCANVAS_H
#define INTERNALLINKCANVAS_H
#include "internallinkgroupbox.h"
#include "Properties/boolproperty.h"

class InternalLinkCanvas : public InternalLinkGroupBox {
    friend class SelfRef;
public:
    void queScheduledTasks();

    void writeBoundingBox(QIODevice *target);

    void readBoundingBox(QIODevice *target);

    void scheduleWaitingTasks();

    void setupBoundingBoxRenderDataForRelFrameF(
                            const qreal &relFrame,
                            BoundingBoxRenderData* data);
    bool clipToCanvas();

    qsptr<BoundingBox> createLinkForLinkGroup();

    stdsptr<BoundingBoxRenderData> createRenderData();

    bool relPointInsidePath(const QPointF &relPos) const;
protected:
    InternalLinkCanvas(BoxesGroup* linkTarget);
    qsptr<BoolProperty> mClipToCanvas =
            SPtrCreate(BoolProperty)("clip");
};

#endif // INTERNALLINKCANVAS_H
