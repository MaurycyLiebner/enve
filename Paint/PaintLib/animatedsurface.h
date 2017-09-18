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

    TilesData *getTilesData() { return mTiles.get(); }
    void setTiles(TilesData *tiles) { mTiles = tiles->ref<TilesData>(); }
    bool differsFromKey(Key *key) { return key != this; }
    void writeSurfaceKey(QFile *file);
    void readSurfaceKey(QFile *file);
    void duplicateTilesContentFrom(Tile ***tilesSrc) {
        mTiles->duplicateTilesContentFrom(tilesSrc);
    }
private:
    std::shared_ptr<TilesData> mTiles;
};

class AnimatedSurface : public Surface,
                        public Animator {
public:
    AnimatedSurface(const ushort &widthT,
                    const ushort &heightT,
                    const qreal &scale,
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

    void writeAnimatedSurface(QFile *file);
    void readAnimatedSurface(QFile *file);
    void currentDataModified();
    bool SWT_isAnimatedSurface() { return true; }
    void anim_saveCurrentValueAsKey();
    void newEmptyPaintFrame();
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    void anim_updateAfterChangedKey(Key *key);
    void newEmptyPaintFrame(const int &relFrame);
    void setOverlapFrames(const int &overlapFrames) {
        mOverlapFrames = overlapFrames;
    }
protected:
    PaintBox *mParentBox = NULL;

    QList<std::shared_ptr<TilesData>> mDrawTilesData;
    QList<int> mDrawTilesFrames;
    int mOverlapFrames = 2;
};

#endif // ANIMATEDSURFACE_H
