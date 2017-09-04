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
        if(mCurrentTiles == NULL) return;
        for(int i = 0; i < mNTileRows; i++) {
            for(int j = 0; j < mNTileCols; j++) {
                delete mCurrentTiles[i][j];
            }
            delete[] mCurrentTiles[i];
        }
        delete[] mCurrentTiles;
    }

    Tile ***getTiles() { return mTiles; }
    void setTiles(Tile ***tiles) { mTiles = tiles; }
    bool differsFromKey(Key *) { return true; }
    void writeSurfaceKey(std::fstream *file, const ushort &nCols, const ushort &nRows);
    void readSurfaceKey(std::fstream *file, const ushort &nCols, const ushort &nRows);
private:
    int mNTileCols = 0;
    int mNTileRows = 0;
    Tile ***mTiles = NULL;
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
    Tile ***createNewTilesArray();
    Tile ***createResizedTiles(const ushort &nTileCols,
                               const ushort &nTilesRows,
                               const ushort &lastColumnWidth,
                               const ushort &lastRowHeight,
                               Tile ***currentTiles);
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
protected:
    PaintBox *mParentBox = NULL;
    int mCurrentTilesFrame = 0;
    int mPreviousTilesFrame = 0;
    Tile ***mNextTiles = NULL;
    int mNextTilesFrame = 0;
};

#endif // ANIMATEDSURFACE_H
