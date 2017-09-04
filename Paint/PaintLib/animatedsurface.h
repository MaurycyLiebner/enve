#ifndef ANIMATEDSURFACE_H
#define ANIMATEDSURFACE_H
#include "surface.h"
#include "Animators/animator.h"
#include "key.h"
class PaintBox;

class SurfaceKey : public Key {
public:
    SurfaceKey(Animator *parentAnimator);
    ~SurfaceKey() {

    }

    void setSize(const ushort &width,
                 const ushort &height) {
        if(mTiles == NULL) {
            mTiles = (new TilesData(width, height, true))->ref<TilesData>();
        } else {
            mTiles->setSize(width, height);
        }
    }

    TilesData *getTiles() { return mTiles.get(); }
    void setTiles(TilesData *tiles) { mTiles = tiles->ref<TilesData>(); }
    bool differsFromKey(Key *key) { return key != this; }
    void writeSurfaceKey(std::fstream *file);
    void readSurfaceKey(std::fstream *file);
private:
    std::shared_ptr<TilesData> mTiles;
};

class AnimatedSurface : public Surface,
                        public Animator {
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

    void anim_removeKey(Key *keyToRemove,
                        const bool &saveUndoRedo = true);
    void anim_appendKey(Key *newKey,
                        const bool &saveUndoRedo = true,
                        const bool &update = true);
    void anim_moveKeyToRelFrame(Key *key,
                                const int &newFrame,
                                const bool &saveUndoRedo = true,
                                const bool &finish = true);

    void writeAnimatedSurface(std::fstream *file);
    void readAnimatedSurface(std::fstream *file);
    void currentDataModified();
protected:
    PaintBox *mParentBox = NULL;
    int mCurrentTilesFrame = 0;
    int mPreviousTilesFrame = 0;
    std::shared_ptr<TilesData> mNextTiles;
    int mNextTilesFrame = 0;
};

#endif // ANIMATEDSURFACE_H
