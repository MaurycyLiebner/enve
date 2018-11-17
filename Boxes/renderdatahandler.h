#ifndef RENDERDATAHANDLER_H
#define RENDERDATAHANDLER_H
#include "boundingboxrenderdata.h"

class RenderDataHandler {
public:
    bool removeItem(const BoundingBoxRenderDataSPtr& item);
    bool removeItemAtRelFrame(const int& frame);
    BoundingBoxRenderDataSPtr getItemAtRelFrame(const int &frame);
    void addItemAtRelFrame(const BoundingBoxRenderDataSPtr& item);
protected:
    int getItemInsertIdAtRelFrame(const int &relFrame);
    bool getItemIdAtRelFrame(const int &relFrame, int *id);
    QList<BoundingBoxRenderDataSPtr > mItems;
};

#endif // RENDERDATAHANDLER_H
