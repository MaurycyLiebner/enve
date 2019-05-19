#ifndef LAYERBOX_H
#define LAYERBOX_H
#include "Boxes/containerbox.h"

class PathBox;
class PathEffectAnimators;

class LayerBox : public ContainerBox {
    friend class SelfRef;
protected:
    LayerBox(const BoundingBoxType& type = TYPE_LAYER);
public:
    bool SWT_isLayerBox() const { return true; }
    //MovablePoint *getPointAt(const QPointF &absPos, const CanvasMode &currentMode);


//    QPointF getRelCenterPosition();
    void drawPixmapSk(SkCanvas * const canvas,
                      GrContext* const grContext);

    stdsptr<BoundingBoxRenderData> createRenderData();
    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData * const data);
    qsptr<BoundingBox> createLink();

    bool shouldPaintOnImage() const;
    void schedulerProccessed();
protected:
    static bool mCtrlsAlwaysVisible;
};

#endif // LAYERBOX_H
