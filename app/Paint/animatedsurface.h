#ifndef ANIMATEDSURFACE_H
#define ANIMATEDSURFACE_H
#include "surface.h"
#include "Animators/animator.h"
#include "key.h"
class PaintBox;

class SurfaceKey : public Key {
    friend class StdSelfRef;
public:
    ~SurfaceKey();

    void setSize(const ushort &width,
                 const ushort &height) {
        if(mTiles == nullptr) {
            mTiles = SPtrCreate(TilesData)(width, height, true);
        } else {
            mTiles->setSize(width, height);
        }
    }

    TilesData *getTilesData() { return mTiles.get(); }
    void setTiles(const stdsptr<TilesData>& tiles) {
        mTiles = tiles;
    }
    bool differsFromKey(Key *key) { return key != this; }
    void writeKey(QIODevice *target);
    void readKey(QIODevice *target);
    void duplicateTilesContentFrom(TilesData *tilesSrc) {
        mTiles->duplicateTilesContentFrom(tilesSrc);
    }
protected:
    SurfaceKey(Animator *parentAnimator);
private:
    stdsptr<TilesData> mTiles;
};

class AnimatedSurface : public Surface,
                        public Animator {
    friend class SelfRef;
public:
    AnimatedSurface(const ushort &widthT,
                    const ushort &heightT,
                    const qreal &scale,
                    PaintBox *parentBox = nullptr);
    ~AnimatedSurface();
    void setCurrentRelFrame(const int &relFrame);

    bool getSurfaceFrameIdBeforeOrAtRelFrame(const int &relFrame,
                                             int *id);
    bool getSurfaceFrameIdAfterRelFrame(const int &relFrame, int *id);
    void getTileDrawers(QList<TileSkDrawerCollection> *tileDrawers);
    void setSize(const ushort &width_t,
                 const ushort &height_t);

    void updateTargetTiles();

    void anim_removeKey(const stdsptr<Key> &keyToRemove,
                        const bool &saveUndoRedo = true);
    void anim_appendKey(const stdsptr<Key> &newKey,
                        const bool &saveUndoRedo = true,
                        const bool &update = true);
    void anim_moveKeyToRelFrame(Key *key,
                                const int &newFrame,
                                const bool &saveUndoRedo = true,
                                const bool &finish = true);

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
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
    stdsptr<Key> readKey(QIODevice *target);
    void move(const int &xT, const int &yT);

    void tabletPressEvent(const qreal &xT,
                          const qreal &yT,
                          const ulong &time_stamp,
                          const qreal &pressure,
                          const bool &erase,
                          const Brush *brush);

    const bool &isDraft() const {
        return mIsDraft;
    }

    void setIsDraft(const bool &bT) {
        mIsDraft = bT;
        updateTargetTiles();
        prp_updateInfluenceRangeAfterChanged();
    }
protected:
    PaintBox *mParentBox = nullptr;

    QList<stdsptr<TilesData>> mDrawTilesData;
    QList<int> mDrawTilesFrames;
    int mOverlapFrames = 2;
    bool mIsDraft = false;
};

#endif // ANIMATEDSURFACE_H
