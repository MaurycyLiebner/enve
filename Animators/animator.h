#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "Properties/property.h"

class ComplexAnimator;
class Key;
class QPainter;
class DurationRectangleMovable;
class FakeComplexAnimator;
typedef QSharedPointer<FakeComplexAnimator> FakeComplexAnimatorQSPtr;

class Animator :
    public Property {
    Q_OBJECT
public:
    Animator();
    ~Animator();

    virtual void anim_updateAfterChangedKey(Key *key);

    virtual void prp_setAbsFrame(const int &frame);

    virtual void prp_switchRecording();

    virtual bool prp_isDescendantRecording() { return anim_mIsRecording; }

    bool anim_isComplexAnimator() { return anim_mIsComplexAnimator; }

    virtual bool prp_isAnimator() { return true; }
    virtual void prp_startDragging() {}

    void anim_updateRelFrame();
    virtual void anim_mergeKeysIfNeeded();
    bool anim_getClosestsKeyOccupiedRelFrame(const int &frame,
                                             int &closest);
    Key *anim_getKeyAtRelFrame(const int &frame);
    bool anim_hasPrevKey(Key *key);
    bool anim_hasNextKey(Key *key);
    virtual void anim_callFrameChangeUpdater();
    virtual void anim_sortKeys();

    virtual void anim_appendKey(Key *newKey,
                                const bool &saveUndoRedo = true,
                                const bool &update = true);
    virtual void anim_removeKey(Key *keyToRemove,
                                const bool &saveUndoRedo = true);
    virtual void anim_moveKeyToRelFrame(Key *key,
                                        const int &newFrame,
                                        const bool &saveUndoRedo = true,
                                        const bool &finish = true);

    virtual void anim_keyValueChanged(Key *key);

    void anim_updateKeyOnCurrrentFrame();

    virtual DurationRectangleMovable *anim_getRectangleMovableAtPos(
                                           const qreal &relX,
                                           const int &minViewedFrame,
                                           const qreal &pixelsPerFrame) {
        Q_UNUSED(relX);
        Q_UNUSED(minViewedFrame);
        Q_UNUSED(pixelsPerFrame);
        return nullptr;
    }

    Key *prp_getKeyAtPos(const qreal &relX,
                         const int &minViewedFrame,
                         const qreal &pixelsPerFrame);
    void prp_removeAllKeysFromComplexAnimator(ComplexAnimator *target);
    void prp_addAllKeysToComplexAnimator(ComplexAnimator *target);
    bool prp_hasKeys();

    void anim_setRecordingWithoutChangingKeys(const bool &rec,
                                              const bool &saveUndoRedo = true);
    bool prp_isRecording();
    virtual void anim_removeAllKeys();
    bool prp_isKeyOnCurrentFrame();
    virtual void prp_getKeysInRect(const QRectF &selectionRect,
                                   const qreal &pixelsPerFrame,
                                   QList<Key *> *keysList);
    bool anim_getNextAndPreviousKeyIdForRelFrame(
                                 int *prevIdP,
                                 int *nextIdP,
                                 const int &frame) const;
    virtual void prp_drawKeys(QPainter *p,
                              const qreal &pixelsPerFrame,
                              const qreal &drawY,
                              const int &startFrame,
                              const int &endFrame);
    Key *anim_getKeyAtAbsFrame(const int &frame);

    virtual void anim_saveCurrentValueAsKey() {}
    virtual void anim_addKeyAtRelFrame(const int &relFrame) { Q_UNUSED(relFrame); }

    Key *anim_getNextKey(Key *key);
    Key *anim_getPrevKey(Key *key);
    int anim_getNextKeyRelFrame(Key *key);
    int anim_getPrevKeyRelFrame(Key *key);
    void anim_setRecordingValue(const bool &rec);

    bool SWT_isAnimator() { return true; }
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    int anim_getCurrentAbsFrame() {
        return anim_mCurrentAbsFrame;
    }
    int anim_getCurrentRelFrame() {
        return anim_mCurrentRelFrame;
    }
    virtual void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                      int *lastIdentical,
                                                      const int &relFrame);
    Key *anim_getPrevKey(const int &relFrame);
    Key *anim_getNextKey(const int &relFrame);
    void anim_shiftAllKeys(const int &shift);

    bool prp_nextRelFrameWithKey(const int &relFrame,
                                 int &nextRelFrame);
    bool prp_prevRelFrameWithKey(const int &relFrame,
                                 int &prevRelFrame);
    virtual Key *readKey(QIODevice *target) {
        Q_UNUSED(target);
        return nullptr;
    }

    bool hasFakeComplexAnimator() {
        return !mFakeComplexAnimator.isNull();
    }

    FakeComplexAnimator *getFakeComplexAnimator() {
        return mFakeComplexAnimator.data();
    }

    void enableFakeComplexAnimator();

    void disableFakeComplexAnimator();
    void disableFakeComplexAnimatrIfNotNeeded();
    int anim_getPrevKeyRelFrame(const int &relFrame);
    int anim_getNextKeyRelFrame(const int &relFrame);
    bool anim_getNextAndPreviousKeyIdForRelFrameF(int *prevIdP,
                                                  int *nextIdP,
                                                  const qreal &frame) const;
protected:
    int anim_getKeyIndex(Key *key);

    virtual void anim_drawKey(QPainter *p,
                         Key *key,
                         const qreal &pixelsPerFrame,
                         const qreal &drawY,
                         const int &startFrame);
    QList<std::shared_ptr<Key> > anim_mKeys;
    int anim_mCurrentAbsFrame = 0;
    int anim_mCurrentRelFrame = 0;
    bool anim_mIsComplexAnimator = false;
    bool anim_mIsCurrentAnimator = false;
    bool anim_mIsRecording = false;
    Key *anim_mKeyOnCurrentFrame = nullptr;
    QColor anim_mAnimatorColor;
    FakeComplexAnimatorQSPtr mFakeComplexAnimator;
public slots:
    void anim_deleteCurrentKey();
    virtual void anim_updateAfterShifted();
    void prp_setRecording(const bool &rec);
};

#endif // ANIMATOR_H
