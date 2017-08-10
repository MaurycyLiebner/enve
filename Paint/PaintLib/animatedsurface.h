#ifndef ANIMATEDSURFACE_H
#define ANIMATEDSURFACE_H
#include "surface.h"

struct SurfaceFrame {
    int relFrame;
    Tile ***tiles = NULL;
};

class AnimatedSurface : public Surface {
public:
    AnimatedSurface(const ushort &widthT,
                    const ushort &heightT,
                    const qreal &scale,
                    const bool &paintOnOtherThread = true);
    void setCurrentRelFrame(const int &relFrame);
    Tile ***getSurfaceFrameBeforeRelFrame(const int &relFrame);
    bool getSurfaceFrameIdBeforeRelFrame(const int &relFrame,
                                         int *id);
protected:
    int mRelFrame = 0;
    void setCurrentSurfaceFrame(const SurfaceFrame &surfaceFrame);
    QList<SurfaceFrame> mSurfaceFrames;
    SurfaceFrame mCurrentSurfaceFrame;
};

#endif // ANIMATEDSURFACE_H
