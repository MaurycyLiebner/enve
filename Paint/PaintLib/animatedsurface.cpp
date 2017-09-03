#include "animatedsurface.h"
#include "Boxes/paintbox.h"

AnimatedSurface::AnimatedSurface(const ushort &widthT,
                                 const ushort &heightT,
                                 const qreal &scale,
                                 const bool &paintOnOtherThread,
                                 PaintBox *parentBox) :
    Surface(widthT, heightT, scale, paintOnOtherThread),
    Animator() {
    mParentBox = parentBox;
    mCurrentTiles = createNewTilesArray();
}

void AnimatedSurface::updateTargetTiles() {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId,
                                               anim_mCurrentRelFrame)) {
        SurfaceKey *prevKey = (SurfaceKey*)anim_mKeys.at(prevId).get();
        mCurrentTiles = prevKey->getTiles();
        mCurrentTilesFrame = prevKey->getRelFrame();
        if(prevId == nextId) {
            mNextTiles = NULL;
            mNextTilesFrame = mCurrentTilesFrame;
        } else {
            SurfaceKey *nextKey = (SurfaceKey*)anim_mKeys.at(nextId).get();
            mNextTiles = nextKey->getTiles();
            mNextTilesFrame = nextKey->getRelFrame();
        }
    }
}

Tile ***AnimatedSurface::createNewTilesArray() {
    ushort lastRowHeight = mHeight%TILE_DIM;
    ushort lastColumnWidth = mWidth%TILE_DIM;
    Tile ***tiles_t = new Tile**[mNTileRows];

    for(ushort rw = 0; rw < mNTileRows; rw++) {
        tiles_t[rw] = new Tile*[mNTileCols];
        for(ushort cl = 0; cl < mNTileCols; cl++) {
            tiles_t[rw][cl] = new Tile(cl*TILE_DIM, rw*TILE_DIM,
                                       mPaintOnOtherThread);
        }
    }
    if(lastRowHeight != 0) {
        for(int i = 0; i < mNTileCols; i++) {
            tiles_t[mNTileRows - 1][i]->setTileHeight(lastRowHeight);
        }
    }
    if(lastColumnWidth != 0) {
        for(int j = 0; j < mNTileRows; j++) {
            tiles_t[j][mNTileCols - 1]->setTileWidth(lastColumnWidth);
        }
    }
    return tiles_t;
}

void AnimatedSurface::setCurrentRelFrame(const int &relFrame) {
    anim_mCurrentRelFrame = relFrame;
    if(mCurrentTilesFrame > relFrame ||
       mNextTilesFrame <= relFrame) {
        updateTargetTiles();
    }
}

void AnimatedSurface::getTileDrawers(QList<TileSkDrawer*> *tileDrawers) {
    if(mCurrentTiles != NULL) {
        int alpha;
        if(mNextTiles == NULL) {
            alpha = 255;
        } else {
            int dR = mNextTilesFrame - mCurrentTilesFrame;
            qreal alphaT = (mCurrentTilesFrame - anim_mCurrentRelFrame)*1./dR;
            alpha = qRound((1. - alphaT*alphaT)*255.);
        }
        for(int i = 0; i < mNTileCols; i++) {
            for(int j = 0; j < mNTileRows; j++) {
                TileSkDrawer *tileDrawer =
                        mCurrentTiles[j][i]->getTexTileDrawer();
                tileDrawer->alpha = alpha;
                tileDrawers->append(tileDrawer);
            }
        }
    }
    if(mNextTiles != NULL) {
        int alpha;
        if(mCurrentTiles == NULL) {
            alpha = 255;
        } else {
            int dR = mNextTilesFrame - mCurrentTilesFrame;
            qreal alphaT = (mNextTilesFrame - anim_mCurrentRelFrame)*1./dR;
            alpha = qRound((1. - alphaT*alphaT)*255.);
        }
        for(int i = 0; i < mNTileCols; i++) {
            for(int j = 0; j < mNTileRows; j++) {
                TileSkDrawer *tileDrawer =
                        mNextTiles[j][i]->getTexTileDrawer();
                tileDrawer->alpha = alpha;
                tileDrawers->append(tileDrawer);
            }
        }
    }
}

Tile ***AnimatedSurface::createResizedTiles(const ushort &nTileCols,
                                         const ushort &nTilesRows,
                                         const ushort &lastColumnWidth,
                                         const ushort &lastRowHeight,
                                         Tile ***currentTiles) {
    Tile ***tiles_t = Surface::createResizedTiles(nTileCols, nTilesRows,
                                                  lastColumnWidth,
                                                  lastRowHeight,
                                                  currentTiles);
    if(currentTiles == mNextTiles) {
        mNextTiles = tiles_t;
    }

    return tiles_t;
}

void AnimatedSurface::anim_removeKey(Key *keyToRemove,
                                     const bool &saveUndoRedo) {
    Animator::anim_removeKey(keyToRemove, saveUndoRedo);
    updateTargetTiles();
}

void AnimatedSurface::anim_appendKey(Key *newKey,
                    const bool &saveUndoRedo,
                    const bool &update) {
    Animator::anim_appendKey(newKey, saveUndoRedo, update);
    updateTargetTiles();
}

void AnimatedSurface::anim_moveKeyToRelFrame(Key *key,
                            const int &newFrame,
                            const bool &saveUndoRedo,
                            const bool &finish) {
    Animator::anim_moveKeyToRelFrame(key, newFrame,
                                     saveUndoRedo, finish);
    updateTargetTiles();
}

void AnimatedSurface::setSize(const ushort &width_t,
                              const ushort &height_t) {
    // initialize tiles
    ushort n_tile_cols_t = ceil(width_t/(qreal)TILE_DIM);
    ushort n_tile_rows_t = ceil(height_t/(qreal)TILE_DIM);
    ushort last_row_height = height_t%TILE_DIM;
    ushort last_column_width = width_t%TILE_DIM;
    if(prp_hasKeys()) {
        Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
            SurfaceKey *frameT = (SurfaceKey*)key.get();
            Tile ***currentTiles = frameT->getTiles();
            Tile ***tiles_t = createResizedTiles(n_tile_cols_t,
                                                 n_tile_rows_t,
                                                 last_column_width,
                                                 last_row_height,
                                                 currentTiles);

            frameT->setTiles(tiles_t);
            delete[] currentTiles;
        }
    } else {
        Tile ***tiles_t = createResizedTiles(n_tile_cols_t,
                                             n_tile_rows_t,
                                             last_column_width,
                                             last_row_height,
                                             mCurrentTiles);

        if(mCurrentTiles != NULL) {
            delete[] mCurrentTiles;
        }
        mCurrentTiles = tiles_t;
    }

    mWidth = width_t;
    mHeight = height_t;
    mNTileRows = n_tile_rows_t;
    mNTileCols = n_tile_cols_t;
}

void AnimatedSurface::newSurfaceFrame() {
    SurfaceKey *prevKey = (SurfaceKey*)anim_getPrevKey(anim_mCurrentRelFrame);
    if(prevKey != NULL) {
        if(prevKey->getRelFrame() == anim_mCurrentRelFrame) {
            clearTiles(prevKey->getTiles());
            return;
        }
    }

    SurfaceKey *frameT = new SurfaceKey(this);
    frameT->setRelFrame(anim_mCurrentRelFrame);
    if(prp_hasKeys()) {
        frameT->setTiles(createNewTilesArray());
    } else {
        frameT->setTiles(mCurrentTiles);
    }

    anim_appendKey(frameT);

    updateTargetTiles();
}

SurfaceKey::SurfaceKey(Animator *parentAnimator) : Key(parentAnimator){

}
