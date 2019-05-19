#ifndef GROUPBOX_H
#define GROUPBOX_H
#include "containerbox.h"

class GroupBox : public ContainerBox {
    friend class SelfRef;
protected:
    GroupBox();
public:
    bool SWT_isGroupBox() const { return true; }

    void drawPixmapSk(SkCanvas * const canvas,
                      GrContext * const grContext);

    stdsptr<BoundingBoxRenderData> createRenderData();
    void setupRenderData(const qreal &relFrame,
                         BoundingBoxRenderData * const data);
};



#endif // GROUPBOX_H
