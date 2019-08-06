#ifndef RENDERDATAHANDLER_H
#define RENDERDATAHANDLER_H
#include "boxrenderdata.h"
#include <map>

class RenderDataHandler {
public:
    void clear() { mFrameToData.clear(); }
    bool removeItem(const stdsptr<BoxRenderData> &item);
    bool removeItemAtRelFrame(const qreal frame);
    BoxRenderData *getItemAtRelFrame(const qreal frame) const;
    void addItemAtRelFrame(const stdsptr<BoxRenderData> &item);
private:
    int frameToKey(const qreal frame) const {
        return qRound(frame*1000);
    }

    std::map<int, stdsptr<BoxRenderData>> mFrameToData;
};

#endif // RENDERDATAHANDLER_H
