#ifndef ANIMATEDSURFACE_H
#define ANIMATEDSURFACE_H
#include "surface.h"
class PaintBox;

struct SurfaceFrame {
    int relFrame;
    Tile ***tiles = NULL;
};

class AnimatedSurface : public Surface {
public:
    AnimatedSurface(const ushort &widthT,
                    const ushort &heightT,
                    const qreal &scale,
                    const bool &paintOnOtherThread = true,
                    PaintBox *parentBox = NULL);
    void setCurrentRelFrame(const int &relFrame);
    Tile ***getTilesBeforeOrAtRelFrame(const int &relFrame,
                                       int *foundAtRelFrame = NULL);
    bool getSurfaceFrameIdBeforeOrAtRelFrame(const int &relFrame,
                                             int *id);
    Tile ***getTilesAfterRelFrame(const int &relFrame,
                                  int *foundAtRelFrame = NULL);
    bool getSurfaceFrameIdAfterRelFrame(const int &relFrame, int *id);
    void getTileDrawers(QList<TileSkDrawer *> *tileDrawers);
    void setSize(const ushort &width_t,
                 const ushort &height_t);

    void newSurfaceFrame();
    void updateTargetTiles();
protected:
    PaintBox *mParentBox = NULL;
    int mRelFrame = 0;
    QList<SurfaceFrame> mSurfaceFrames;
    int mCurrentTilesFrame = 0;
    Tile ***mPreviousTiles = NULL;
    int mPreviousTilesFrame = 0;
    Tile ***mNextTiles = NULL;
    int mNextTilesFrame = 0;
};

#endif // ANIMATEDSURFACE_H
