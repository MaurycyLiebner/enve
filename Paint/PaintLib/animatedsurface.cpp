#include "animatedsurface.h"
#include "Boxes/paintbox.h"

void getTileDrawers(const std::shared_ptr<TilesData> &drawTilesData,
                    const int &drawTilesFrame,
                    const int &neighDrawerRelFrame,
                    const int &currRelFrame,
                    const int &additionalFrames,
                    QList<TileSkDrawer*> *tileDrawers) {
    int alpha;
    if(neighDrawerRelFrame == INT_MIN ||
       neighDrawerRelFrame == INT_MAX) {
        alpha = 255;
    } else {
        int dR = qAbs(drawTilesFrame - neighDrawerRelFrame) + additionalFrames;
        int dFrame = drawTilesFrame - currRelFrame;
        qreal alphaT = dFrame*1./dR;
        alpha = qMin(255, qMax(0, qRound((1. - alphaT*alphaT)*255.)) );
    }
    drawTilesData->getTileDrawers(tileDrawers, alpha);
}

AnimatedSurface::AnimatedSurface(const ushort &widthT,
                                 const ushort &heightT,
                                 const qreal &scale,
                                 const bool &paintOnOtherThread,
                                 PaintBox *parentBox) :
    Surface(widthT, heightT, scale, paintOnOtherThread),
    Animator() {
    prp_setName("canvas");
    mParentBox = parentBox;
}

void AnimatedSurface::currentDataModified() {
    if(prp_hasKeys()) {
        if(prp_isKeyOnCurrentFrame()) {
            anim_updateAfterChangedKey(anim_mKeyOnCurrentFrame);
        } else {
            Key *key = anim_getPrevKey(anim_mCurrentRelFrame);
            if(key == NULL) {
                key = anim_getNextKey(anim_mCurrentRelFrame);
            }
            anim_updateAfterChangedKey(key);
        }
    } else {
        prp_updateInfluenceRangeAfterChanged();
    }
}

void AnimatedSurface::anim_saveCurrentValueAsKey() {
    if(prp_isKeyOnCurrentFrame()) return;
    newEmptyPaintFrame();
    SurfaceKey *key = (SurfaceKey*)anim_mKeyOnCurrentFrame;
    SurfaceKey *prevKey = (SurfaceKey*)anim_getPrevKey(key);
    Tile ***tiles;
    if(prevKey == NULL) {
        tiles = mCurrentTiles->getData();
    } else {
        tiles = prevKey->getTiles()->getData();
    }
    key->duplicateTilesContentFrom(tiles);
}

void AnimatedSurface::newEmptyPaintFrame() {
    if(!anim_mIsRecording) {
        prp_setRecording(true);
        return;
    }

    SurfaceKey *prevKey = (SurfaceKey*)anim_getPrevKey(anim_mCurrentRelFrame);
    if(prevKey != NULL) {
        if(prevKey->getRelFrame() == anim_mCurrentRelFrame) {
            prevKey->getTiles()->clearTiles();
            return;
        }
    }

    SurfaceKey *frameT = new SurfaceKey(this);
    frameT->setRelFrame(anim_mCurrentRelFrame);
    if(prp_hasKeys()) {
        frameT->setSize(mWidth, mHeight);
    } else {
        frameT->setTiles(mCurrentTiles.get());
    }

    anim_appendKey(frameT);

    updateTargetTiles();
}

void AnimatedSurface::updateTargetTiles() {
    mDrawTilesData.clear();
    mDrawTilesFrames.clear();
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId,
                                               anim_mCurrentRelFrame)) {
        SurfaceKey *prevKey = (SurfaceKey*)anim_mKeys.at(prevId).get();
        mDrawTilesData.append(prevKey->getTiles()->ref<TilesData>());
        mDrawTilesFrames.append(prevKey->getRelFrame());
        int idT = prevId - 1;
        while(idT >= 0) {
            SurfaceKey *keyT = (SurfaceKey*)anim_mKeys.at(idT).get();
            mDrawTilesData.prepend(keyT->getTiles()->ref<TilesData>());
            mDrawTilesFrames.prepend(keyT->getRelFrame());
            idT--;
            if(anim_mCurrentRelFrame - keyT->getRelFrame() >=
                    mAdditionalFrames) break;
        }
        SurfaceKey *nextKey = (SurfaceKey*)anim_mKeys.at(nextId).get();
        if(nextKey != prevKey) {
            mDrawTilesData.append(nextKey->getTiles()->ref<TilesData>());
            mDrawTilesFrames.append(nextKey->getRelFrame());
        }
        idT = nextId + 1;
        while(idT < anim_mKeys.count()) {
            SurfaceKey *keyT = (SurfaceKey*)anim_mKeys.at(idT).get();
            mDrawTilesData.append(keyT->getTiles()->ref<TilesData>());
            mDrawTilesFrames.append(keyT->getRelFrame());
            idT++;
            if(keyT->getRelFrame() - anim_mCurrentRelFrame >=
                    mAdditionalFrames) break;
        }

        int nextDFrame = qAbs(nextKey->getRelFrame() - anim_mCurrentRelFrame);
        int prevDFrame = qAbs(prevKey->getRelFrame() - anim_mCurrentRelFrame);
        if(nextDFrame > prevDFrame) {
            mCurrentTiles = prevKey->getTiles()->ref<TilesData>();
        } else {
            mCurrentTiles = nextKey->getTiles()->ref<TilesData>();
        }
    }
}

void AnimatedSurface::setCurrentRelFrame(const int &relFrame) {
    anim_mCurrentRelFrame = relFrame;
    updateTargetTiles();
}

void AnimatedSurface::getTileDrawers(QList<TileSkDrawer*> *tileDrawers) {
    if(mDrawTilesFrames.isEmpty()) {
        ::getTileDrawers(mCurrentTiles, 0,
                         INT_MIN, anim_mCurrentRelFrame,
                         mAdditionalFrames,
                         tileDrawers);
    } else {
        int countT = mDrawTilesFrames.count();
        int prevRelFrame = INT_MIN;
        for(int i = 0; i < countT; i++) {
            const int &relFrame = mDrawTilesFrames.at(i);
            std::shared_ptr<TilesData> tilesData = mDrawTilesData.at(i);
            int neighRelFrame;
            if(relFrame < anim_mCurrentRelFrame) {
                if(i + 1 < countT) {
                    neighRelFrame = mDrawTilesFrames.at(i + 1);
                } else {
                    neighRelFrame = INT_MAX;
                }

            } else {
                neighRelFrame = prevRelFrame;
            }
            ::getTileDrawers(tilesData, relFrame,
                             neighRelFrame, anim_mCurrentRelFrame,
                             mAdditionalFrames,
                             tileDrawers);

            prevRelFrame = relFrame;
        }
    }
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

    if(prp_hasKeys()) {
        ushort n_tile_cols_t = ceil(width_t/(qreal)TILE_DIM);
        ushort n_tile_rows_t = ceil(height_t/(qreal)TILE_DIM);
        Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
            SurfaceKey *frameT = (SurfaceKey*)key.get();
            frameT->setSize(width_t,
                            height_t);
        }
        mWidth = width_t;
        mHeight = height_t;
        mNTileRows = n_tile_rows_t;
        mNTileCols = n_tile_cols_t;
    } else {
        Surface::setSize(width_t, height_t);
    }
}

bool AnimatedSurface::prp_differencesBetweenRelFrames(const int &relFrame1,
                                                      const int &relFrame2) {
    if(prp_hasKeys()) {
        int firstKeyRelFrame = anim_mKeys.first()->getRelFrame();
        int lastKeyRelFrame = anim_mKeys.last()->getRelFrame();
        if(relFrame1 >= lastKeyRelFrame + mAdditionalFrames &&
           relFrame2 >= lastKeyRelFrame + mAdditionalFrames) return false;
        if(relFrame1 <= firstKeyRelFrame - mAdditionalFrames &&
           relFrame2 <= firstKeyRelFrame - mAdditionalFrames) return false;
        return true;
    }
    return false;
}

void AnimatedSurface::prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                           int *lastIdentical,
                                                           const int &relFrame) {
    Animator::prp_getFirstAndLastIdenticalRelFrame(
                                firstIdentical,
                                lastIdentical,
                                relFrame);
    if(prp_hasKeys()) {
        if(*firstIdentical == INT_MIN) {
            int firstKeyFrame = anim_mKeys.first()->getRelFrame();
            if(firstKeyFrame - mAdditionalFrames < relFrame) {
                *firstIdentical = relFrame;
                *lastIdentical = relFrame;
            } else {
                *lastIdentical = firstKeyFrame - mAdditionalFrames;
            }
        } else if(*lastIdentical == INT_MAX) {
            int lastKeyFrame = anim_mKeys.last()->getRelFrame();
            if(lastKeyFrame + mAdditionalFrames > relFrame) {
                *firstIdentical = relFrame;
                *lastIdentical = relFrame;
            } else {
                *firstIdentical = lastKeyFrame + mAdditionalFrames;
            }
        }
    }
}

SurfaceKey::SurfaceKey(Animator *parentAnimator) :
    Key(parentAnimator){
}
