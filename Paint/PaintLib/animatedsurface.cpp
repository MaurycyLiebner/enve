#include "animatedsurface.h"
#include "Boxes/paintbox.h"

AnimatedSurface::AnimatedSurface(const ushort &widthT,
                                 const ushort &heightT,
                                 const qreal &scale,
                                 const bool &paintOnOtherThread,
                                 PaintBox *parentBox) :
    Surface(widthT, heightT, scale, paintOnOtherThread) {
    mParentBox = parentBox;
    mRelFrame = mParentBox->anim_getCurrentRelFrame();
    newSurfaceFrame();
}

void AnimatedSurface::updateTargetTiles() {
    mCurrentTiles = getTilesBeforeOrAtRelFrame(mRelFrame,
                               &mCurrentTilesFrame);
    mPreviousTiles = getTilesBeforeOrAtRelFrame(mCurrentTilesFrame - 1,
                                                &mPreviousTilesFrame);
    mNextTiles = getTilesAfterRelFrame(mRelFrame,
                                       &mNextTilesFrame);
    mParentBox->scheduleUpdate();
}

void AnimatedSurface::setCurrentRelFrame(const int &relFrame) {
    mRelFrame = relFrame;
    if(mCurrentTilesFrame <= relFrame &&
       mNextTilesFrame > relFrame) return;
    updateTargetTiles();
}

Tile*** AnimatedSurface::getTilesBeforeOrAtRelFrame(const int &relFrame,
                                                    int *foundAtRelFrame) {
    int id;
    if(getSurfaceFrameIdBeforeOrAtRelFrame(relFrame, &id)) {
        const SurfaceFrame &frameT = mSurfaceFrames.at(id);
        if(foundAtRelFrame != NULL) *foundAtRelFrame = frameT.relFrame;
        return frameT.tiles;
    }
    return NULL;
}

bool AnimatedSurface::getSurfaceFrameIdBeforeOrAtRelFrame(const int &relFrame,
                                                          int *id) {
    int minId = 0;
    int maxId = mSurfaceFrames.count() - 1;

    while(minId <= maxId) {
        int guess = (minId + maxId)/2;
        const SurfaceFrame &frameT = mSurfaceFrames.at(guess);
        if(frameT.relFrame == relFrame) {
            *id = guess;
            return true;
        }
        if(frameT.relFrame > relFrame) {
            maxId = guess - 1;
            continue;
        }
        if(guess == mSurfaceFrames.count() - 1) {
            *id = guess;
            return true;
        }
        const SurfaceFrame &nextFrameT = mSurfaceFrames.at(guess + 1);
        if(nextFrameT.relFrame > relFrame) {
            *id = guess;
            return true;
        } else if(nextFrameT.relFrame == relFrame) {
            *id = guess + 1;
            return true;
        }
        minId = guess + 1;
    }
    return false;
}

Tile*** AnimatedSurface::getTilesAfterRelFrame(const int &relFrame,
                                               int *foundAtRelFrame) {
    int id;
    if(getSurfaceFrameIdAfterRelFrame(relFrame, &id)) {
        const SurfaceFrame &frameT = mSurfaceFrames.at(id);
        if(foundAtRelFrame != NULL) *foundAtRelFrame = frameT.relFrame;
        return frameT.tiles;
    }
    return NULL;
}

bool AnimatedSurface::getSurfaceFrameIdAfterRelFrame(const int &relFrame,
                                                     int *id) {
    int minId = 0;
    int maxId = mSurfaceFrames.count() - 1;

    while(minId <= maxId) {
        int guess = (minId + maxId)/2;
        const SurfaceFrame &frameT = mSurfaceFrames.at(guess);
        if(frameT.relFrame <= relFrame) {
            minId = guess + 1;
            continue;
        }
        if(guess == 0) {
            *id = guess;
            return true;
        }
        const SurfaceFrame &prevFrameT = mSurfaceFrames.at(guess - 1);
        if(prevFrameT.relFrame <= relFrame) {
            *id = guess;
            return true;
        }
        maxId = guess - 1;
    }
    return false;
}

void AnimatedSurface::getTileDrawers(QList<TileSkDrawer*> *tileDrawers) {
    int frameDist = mRelFrame - mCurrentTilesFrame;

    if(mCurrentTiles != NULL) {
        qreal dR = (mNextTilesFrame - mCurrentTilesFrame)*0.5;
        int alpha = 255 - qMin(255, qMax(0,
                255 - qRound((frameDist - dR)*255/dR)) );
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
        qreal dR = (mNextTilesFrame - mCurrentTilesFrame)*0.5;
        int alpha = qMin(255, qMax(0,
                qRound((frameDist - dR)*255/dR )));
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


void AnimatedSurface::setSize(const ushort &width_t,
                              const ushort &height_t) {
    // initialize tiles
    ushort n_tile_cols_t = ceil(width_t/(qreal)TILE_DIM);
    ushort n_tile_rows_t = ceil(height_t/(qreal)TILE_DIM);
    ushort last_row_height = mHeight%TILE_DIM;
    ushort last_column_width = mWidth%TILE_DIM;
    int id = 0;
    foreach(const SurfaceFrame &frameT, mSurfaceFrames) {
        Tile ***currentTiles = frameT.tiles;
        Tile ***tiles_t = new Tile**[n_tile_rows_t];

        for(ushort rw = 0; rw < n_tile_rows_t; rw++) {
            tiles_t[rw] = new Tile*[n_tile_cols_t];
            ushort first_new_col_in_row = 0;
            if(rw < mNTileRows) {
                first_new_col_in_row = mNTileCols;
                for(ushort cl = 0; cl < mNTileCols; cl++) {
                    Tile *tile_t = currentTiles[rw][cl];
                    if(cl < n_tile_cols_t) {
                        tile_t->resetTileSize();
                        tiles_t[rw][cl] = tile_t;
                    } else {
                        delete tile_t;
                    }
                }
                delete[] currentTiles[rw];
            }

            for(ushort cl = first_new_col_in_row; cl < n_tile_cols_t; cl++) {
                tiles_t[rw][cl] = new Tile(cl*TILE_DIM, rw*TILE_DIM,
                                           mPaintOnOtherThread);
            }

        }

        if(currentTiles == mCurrentTiles) {
            mCurrentTiles = tiles_t;
        } else if(currentTiles == mPreviousTiles) {
            mPreviousTiles = tiles_t;
        } else if(currentTiles == mNextTiles) {
            mNextTiles = tiles_t;
        }

        if(currentTiles != NULL) {
            delete[] currentTiles;
        }
        SurfaceFrame newFrame = frameT;
        newFrame.tiles = tiles_t;
        mSurfaceFrames.replace(id, newFrame);

        if(last_row_height != 0) {
            for(int i = 0; i < mNTileCols; i++) {
                tiles_t[mNTileRows - 1][i]->setTileHeight(last_row_height);
            }
        }
        if(last_column_width != 0) {
            for(int j = 0; j < mNTileRows; j++) {
                tiles_t[j][mNTileCols - 1]->setTileWidth(last_column_width);
            }
        }
        id++;
    }

    mWidth = width_t;
    mHeight = height_t;
    mNTileRows = n_tile_rows_t;
    mNTileCols = n_tile_cols_t;
}

void AnimatedSurface::newSurfaceFrame() {
    int frameBefore;
    Tile ***tilesBefore =
            getTilesBeforeOrAtRelFrame(mRelFrame, &frameBefore);
    if(tilesBefore != NULL) {
        if(frameBefore == mRelFrame) {
            clearTiles(tilesBefore);
            return;
        }
    }
    ushort last_row_height = mHeight%TILE_DIM;
    ushort last_column_width = mWidth%TILE_DIM;
    Tile ***tiles_t = new Tile**[mNTileRows];

    SurfaceFrame frameT;
    frameT.relFrame = mRelFrame;
    frameT.tiles = tiles_t;

    for(ushort rw = 0; rw < mNTileRows; rw++) {
        tiles_t[rw] = new Tile*[mNTileCols];
        for(ushort cl = 0; cl < mNTileCols; cl++) {
            tiles_t[rw][cl] = new Tile(cl*TILE_DIM, rw*TILE_DIM,
                                       mPaintOnOtherThread);
        }

    }

    if(last_row_height != 0) {
        for(int i = 0; i < mNTileCols; i++) {
            tiles_t[mNTileRows - 1][i]->setTileHeight(last_row_height);
        }
    }
    if(last_column_width != 0) {
        for(int j = 0; j < mNTileRows; j++) {
            tiles_t[j][mNTileCols - 1]->setTileWidth(last_column_width);
        }
    }

    int id;
    if(getSurfaceFrameIdAfterRelFrame(mRelFrame, &id)) {
        mSurfaceFrames.insert(id, frameT);
    } else {
        mSurfaceFrames.append(frameT);
    }
    updateTargetTiles();
}
