#include "animatedsurface.h"
#include "Boxes/paintbox.h"

void getTileDrawers(const stdsptr<TilesData> &drawTilesData,
                    const int &drawTilesFrame,
                    const int &neighDrawerRelFrame,
                    const int &currRelFrame,
                    const int &additionalFrames,
                    QList<TileSkDrawerCollection> *tileDrawers) {
    int alpha;
    if(neighDrawerRelFrame == FrameRange::EMIN ||
       neighDrawerRelFrame == FrameRange::EMAX) {
        alpha = 255;
    } else {
        int dR = qAbs(drawTilesFrame - neighDrawerRelFrame) + additionalFrames;
        int dFrame = qAbs(drawTilesFrame - currRelFrame);
        qreal alphaT = dFrame*1./dR;
        alpha = qMin(255, qMax(0, qRound((1. - alphaT/*alphaT*/)*255.)) );
    }
    TileSkDrawerCollection coll;
    coll.alpha = static_cast<uchar>(alpha);
    drawTilesData->getTileDrawers(&coll.drawers);
    tileDrawers->append(coll);
}

AnimatedSurface::AnimatedSurface(const ushort &widthT,
                                 const ushort &heightT,
                                 const qreal &scale,
                                 PaintBox *parentBox) :
    Surface(widthT, heightT, scale, true),
    Animator("canvas") {
    mParentBox = parentBox;
}

AnimatedSurface::~AnimatedSurface() {}

void AnimatedSurface::currentDataModified() {
    if(anim_hasKeys()) {
        const auto currKey = anim_getKeyOnCurrentFrame();
        if(currKey) {
            anim_updateAfterChangedKey(currKey);
        } else {
            Key *key = anim_getPrevKey(anim_mCurrentRelFrame);
            if(!key) {
                key = anim_getNextKey(anim_mCurrentRelFrame);
            }
            anim_updateAfterChangedKey(key);
        }
    } else {
        prp_updateInfluenceRangeAfterChanged();
    }
}

void AnimatedSurface::anim_saveCurrentValueAsKey() {
    if(anim_getKeyOnCurrentFrame()) return;
    newEmptyPaintFrame();
    const auto key = anim_getKeyOnCurrentFrame<SurfaceKey>();
    SurfaceKey *prevKey = anim_getPrevKey<SurfaceKey>(key);
    TilesData *tiles;
    if(!prevKey) {
        tiles = mCurrentTiles.get();
    } else {
        tiles = prevKey->getTilesData();
    }
    key->duplicateTilesContentFrom(tiles);
}

void AnimatedSurface::newEmptyPaintFrame(const int &relFrame) {
    if(!anim_mIsRecording) {
        anim_setRecordingValue(true);
    }
    Key *keyAtFrame = anim_getKeyAtRelFrame(relFrame);
    if(keyAtFrame) {
        GetAsPtr(keyAtFrame, SurfaceKey)->getTilesData()->clearTiles();
        return;
    }

    SurfaceKey *prevKey = GetAsPtr(anim_getPrevKey(relFrame), SurfaceKey);
    if(prevKey) {
        if(prevKey->getRelFrame() == relFrame) {
            prevKey->getTilesData()->clearTiles();
            return;
        }
    }

    auto frameT = SPtrCreate(SurfaceKey)(relFrame, this);
    if(anim_hasKeys()) {
        frameT->setSize(static_cast<ushort>(mWidth),
                        static_cast<ushort>(mHeight));
    } else {
        frameT->setTiles(mCurrentTiles);
    }

    anim_appendKey(frameT);

    updateTargetTiles();
}

void AnimatedSurface::newEmptyPaintFrame() {
    newEmptyPaintFrame(anim_mCurrentRelFrame);
}

void AnimatedSurface::updateTargetTiles() {
    mCurrentTiles->setCurrentlyUsed(false);
    for(const auto& tilesDataT : mDrawTilesData) {
        tilesDataT->setCurrentlyUsed(false);
    }
    mDrawTilesData.clear();
    mDrawTilesFrames.clear();
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId,
                                               anim_mCurrentRelFrame)) {
        SurfaceKey *prevKey = GetAsPtr(anim_mKeys.at(prevId), SurfaceKey);
        SurfaceKey *nextKey = GetAsPtr(anim_mKeys.at(nextId), SurfaceKey);

        TilesData *prevKeyTilesData = prevKey->getTilesData();
        prevKeyTilesData->setCurrentlyUsed(true);
        mDrawTilesData.append(GetAsSPtr(prevKeyTilesData, TilesData));
        mDrawTilesFrames.append(prevKey->getRelFrame());
        if(mIsDraft) {
            int idT = prevId - 1;
            while(idT >= 0) {
                SurfaceKey *keyT = GetAsPtr(anim_mKeys.at(idT), SurfaceKey);
                TilesData *keyTTilesData = keyT->getTilesData();
                keyTTilesData->setCurrentlyUsed(true);
                mDrawTilesData.prepend(GetAsSPtr(keyTTilesData, TilesData));
                mDrawTilesFrames.prepend(keyT->getRelFrame());
                idT--;
                if(anim_mCurrentRelFrame - keyT->getRelFrame() >=
                        mOverlapFrames) break;
            }
            if(nextKey != prevKey) {
                TilesData *nextKeyTilesData = nextKey->getTilesData();
                nextKeyTilesData->setCurrentlyUsed(true);
                mDrawTilesData.append(GetAsSPtr(nextKeyTilesData, TilesData));
                mDrawTilesFrames.append(nextKey->getRelFrame());
            }
            idT = nextId + 1;
            while(idT < anim_mKeys.count()) {
                SurfaceKey *keyT = GetAsPtr(anim_mKeys.at(idT), SurfaceKey);
                TilesData *keyTTilesData = keyT->getTilesData();
                keyTTilesData->setCurrentlyUsed(true);
                mDrawTilesData.append(GetAsSPtr(keyTTilesData, TilesData));
                mDrawTilesFrames.append(keyT->getRelFrame());
                idT++;
                if(keyT->getRelFrame() - anim_mCurrentRelFrame >=
                        mOverlapFrames) break;
            }
        }

        int nextDFrame = qAbs(nextKey->getRelFrame() - anim_mCurrentRelFrame);
        int prevDFrame = qAbs(prevKey->getRelFrame() - anim_mCurrentRelFrame);

        if(nextDFrame > prevDFrame) {
            mCurrentTiles = GetAsSPtr(prevKey->getTilesData(), TilesData);
        } else {
            mCurrentTiles = GetAsSPtr(nextKey->getTilesData(), TilesData);
        }
    }
    mCurrentTiles->setCurrentlyUsed(true);
}

void AnimatedSurface::setCurrentRelFrame(const int &relFrame) {
    anim_mCurrentRelFrame = relFrame;
    updateTargetTiles();
}

void AnimatedSurface::getTileDrawers(QList<TileSkDrawerCollection> *tileDrawers) {
    if(mDrawTilesFrames.isEmpty()) {
        TileSkDrawerCollection coll;
        mCurrentTiles->getTileDrawers(&coll.drawers);
        tileDrawers->append(coll);
    } else {
        int countT = mDrawTilesFrames.count();
        int prevRelFrame = FrameRange::EMIN;
        for(int i = 0; i < countT; i++) {
            const int &relFrame = mDrawTilesFrames.at(i);
            stdsptr<TilesData> tilesData = mDrawTilesData.at(i);
            int neighRelFrame;
            if(relFrame < anim_mCurrentRelFrame) {
                if(i + 1 < countT) {
                    neighRelFrame = mDrawTilesFrames.at(i + 1);
                } else {
                    neighRelFrame = FrameRange::EMAX;
                }

            } else {
                neighRelFrame = prevRelFrame;
            }
//            ::getTileDrawers(tilesData, relFrame,
//                             neighRelFrame, anim_mCurrentRelFrame,
//                             mOverlapFrames,
//                             tileDrawers);

            TileSkDrawerCollection coll;
            tilesData->getTileDrawers(&coll.drawers);
            if(mIsDraft) {
                int alpha;
                if(neighRelFrame == FrameRange::EMIN ||
                   neighRelFrame == FrameRange::EMAX) {
                    alpha = 255;
                } else {
    //                int dR = qAbs(relFrame - neighRelFrame) + mOverlapFrames;
    //                int dFrame = qAbs(relFrame - anim_mCurrentRelFrame);
    //                qreal alphaT = dFrame*1./dR;
    //                alpha = qMin(255, qMax(0, qRound((1. - alphaT/*alphaT*/)*255.)) );
                    qreal alphaT = 0.;
                    if((relFrame < neighRelFrame &&
                        anim_mCurrentRelFrame >= neighRelFrame) ||
                        (relFrame > neighRelFrame &&
                         anim_mCurrentRelFrame <= neighRelFrame)) {
                        alphaT = (qAbs(anim_mCurrentRelFrame - neighRelFrame) + 1)*1./(mOverlapFrames + 1);
                    }
                    alpha = qMin(255, qMax(0, qRound((1. - alphaT/*alphaT*/)*255.)) );
                }
                coll.alpha = static_cast<uchar>(alpha);
                bool hueChange = relFrame != anim_mCurrentRelFrame;
                coll.setHueChangeEnabled(hueChange);

                if(hueChange) {
                    coll.setHue(relFrame > anim_mCurrentRelFrame ? 0. : 0.5);
                }
            }

            tileDrawers->append(coll);

            prevRelFrame = relFrame;
        }
    }
}

void AnimatedSurface::anim_removeKey(const stdsptr<Key>& keyToRemove) {
    Animator::anim_removeKey(keyToRemove);
    updateTargetTiles();
}

void AnimatedSurface::anim_appendKey(const stdsptr<Key>& newKey) {
    Animator::anim_appendKey(newKey);
    updateTargetTiles();
}

void AnimatedSurface::anim_moveKeyToRelFrame(Key *key,
                            const int &newFrame) {
    Animator::anim_moveKeyToRelFrame(key, newFrame);
    updateTargetTiles();
}

void AnimatedSurface::setSize(const ushort &width_t,
                              const ushort &height_t) {
    // initialize tiles
    if(width_t == mWidth && height_t == mHeight) return;
    if(anim_hasKeys()) {
        int n_tile_cols_t = qCeil(width_t/static_cast<qreal>(TILE_DIM));
        int n_tile_rows_t = qCeil(height_t/static_cast<qreal>(TILE_DIM));
        for(const auto &key : anim_mKeys) {
            SurfaceKey *frameT = GetAsPtr(key, SurfaceKey);
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

void AnimatedSurface::move(const int &xT, const int &yT) {
    if(anim_hasKeys()) {
        for(const auto &key : anim_mKeys) {
            SurfaceKey *frameT = GetAsPtr(key, SurfaceKey);
            frameT->getTilesData()->move(xT, yT);
        }
    } else {
        Surface::move(xT, yT);
    }
}

void AnimatedSurface::tabletPressEvent(const qreal &xT, const qreal &yT,
                                       const ulong &time_stamp,
                                       const qreal &pressure,
                                       const bool &erase,
                                       const Brush *brush) {
    if(anim_isRecording() && !anim_getKeyOnCurrentFrame()) {
        newEmptyPaintFrame();
    }
    Surface::tabletPressEvent(xT, yT, time_stamp,
                              pressure, erase, brush);
}

void AnimatedSurface::anim_updateAfterChangedKey(Key * const key) {
    if(SWT_isComplexAnimator()) return;
    if(!key) {
        prp_updateInfluenceRangeAfterChanged();
        return;
    }
    int prevKeyRelFrame = anim_getPrevKeyRelFrame(key);
    if(prevKeyRelFrame != FrameRange::EMIN) {
        prevKeyRelFrame++;
        prevKeyRelFrame -= mOverlapFrames;
    }
    int nextKeyRelFrame = anim_getNextKeyRelFrame(key);
    if(nextKeyRelFrame != FrameRange::EMAX) {
        nextKeyRelFrame--;
        nextKeyRelFrame += mOverlapFrames;
    }

    prp_updateAfterChangedRelFrameRange({prevKeyRelFrame, nextKeyRelFrame});
}

FrameRange AnimatedSurface::prp_getIdenticalRelFrameRange(const int &relFrame) const {
    auto range = Animator::prp_getIdenticalRelFrameRange(relFrame);
    if(!mIsDraft) return range;
    if(anim_mKeys.count() > 1) {
        if(range.fMin == FrameRange::EMIN) {
            int firstKeyFrame = anim_mKeys.first()->getRelFrame();
            if(firstKeyFrame - mOverlapFrames < relFrame) {
                return {relFrame, relFrame};
            } else {
                return {range.fMin, firstKeyFrame - mOverlapFrames};
            }
        } else if(range.fMax == FrameRange::EMAX) {
            int lastKeyFrame = anim_mKeys.last()->getRelFrame();
            if(lastKeyFrame + mOverlapFrames > relFrame) {
                return {relFrame, relFrame};
            } else {
                return {lastKeyFrame + mOverlapFrames, range.fMax};
            }
        }
    }
    return range;
}

SurfaceKey::SurfaceKey(Animator * const parentAnimator) :
    Key(parentAnimator) {}

SurfaceKey::SurfaceKey(const int &relFrame,
                       Animator * const parentAnimator) :
    Key(relFrame, parentAnimator) {}

SurfaceKey::~SurfaceKey() {}
