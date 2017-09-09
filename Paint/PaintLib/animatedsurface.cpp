#include "animatedsurface.h"
#include "Boxes/paintbox.h"

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
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId,
                                               anim_mCurrentRelFrame)) {
        SurfaceKey *prevKey = (SurfaceKey*)anim_mKeys.at(prevId).get();
        mCurrentTiles = prevKey->getTiles()->ref<TilesData>();
        mCurrentTilesFrame = prevKey->getRelFrame();
        if(prevId == nextId) {
            mNextTiles = NULL;
            mNextTilesFrame = mCurrentTilesFrame;
        } else {
            SurfaceKey *nextKey = (SurfaceKey*)anim_mKeys.at(nextId).get();
            mNextTiles = nextKey->getTiles()->ref<TilesData>();
            mNextTilesFrame = nextKey->getRelFrame();
        }
    }
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
        mCurrentTiles->getTileDrawers(tileDrawers, alpha);
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
        mNextTiles->getTileDrawers(tileDrawers, alpha);
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

SurfaceKey::SurfaceKey(Animator *parentAnimator) :
    Key(parentAnimator){
}
