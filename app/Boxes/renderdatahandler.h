#ifndef RENDERDATAHANDLER_H
#define RENDERDATAHANDLER_H
#include "boundingboxrenderdata.h"

class RenderDataHandler {
public:
    void clear() { mItems.clear(); }

    bool removeItem(const stdsptr<BoundingBoxRenderData> &item);
    bool removeItemAtRelFrame(const int& frame);
    BoundingBoxRenderData *getItemAtRelFrame(const int &frame);
    void addItemAtRelFrame(const stdsptr<BoundingBoxRenderData> &item);
protected:
    int getItemInsertIdAtRelFrame(const int &relFrame);
    bool getItemIdAtRelFrame(const int &relFrame, int *id);
    QList<stdsptr<BoundingBoxRenderData> > mItems;
};

#endif // RENDERDATAHANDLER_H
