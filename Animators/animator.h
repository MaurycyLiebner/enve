#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "Properties/property.h"

class ComplexAnimator;
class Key;
class QrealAnimator;
class QPainter;
class AnimatorUpdater;
class DurationRectangleMovable;

class Animator :
    public Property {
    Q_OBJECT
public:
    Animator();
    ~Animator();

    virtual void prp_setAbsFrame(int frame);

    virtual void prp_switchRecording();

    virtual bool prp_isDescendantRecording() { return anim_mIsRecording; }

    bool anim_isComplexAnimator() { return anim_mIsComplexAnimator; }

    virtual bool prp_isAnimator() { return true; }
    virtual void prp_startDragging() {}

    virtual int anim_getFrameShift() const;
    virtual int anim_getParentFrameShift() const;

    void anim_updateRelFrame();
    int anim_absFrameToRelFrame(const int &absFrame) const;
    int anim_relFrameToAbsFrame(const int &relFrame) const;
    virtual void anim_mergeKeysIfNeeded();
    int anim_getClosestsKeyOccupiedRelFrame(const int &frame);
    Key *anim_getKeyAtRelFrame(const int &frame);
    bool anim_hasPrevKey(Key *key);
    bool anim_hasNextKey(Key *key);
    virtual void anim_callFrameChangeUpdater();
    virtual void anim_sortKeys();

    virtual void anim_appendKey(Key *newKey,
                                bool saveUndoRedo = true);
    virtual void anim_removeKey(Key *keyToRemove,
                                bool saveUndoRedo = true);
    virtual void anim_moveKeyToFrame(Key *key, int newFrame);
    void anim_updateKeyOnCurrrentFrame();
    void anim_setTraceKeyOnCurrentFrame(bool bT) {
        anim_mTraceKeyOnCurrentFrame = bT;
    }
    virtual DurationRectangleMovable *anim_getRectangleMovableAtPos(qreal relX,
                                                       int minViewedFrame,
                                                       qreal pixelsPerFrame) {
        Q_UNUSED(relX);
        Q_UNUSED(minViewedFrame);
        Q_UNUSED(pixelsPerFrame);
        return NULL;
    }

    Key *prp_getKeyAtPos(qreal relX,
                         int minViewedFrame,
                         qreal pixelsPerFrame);
    void prp_removeAllKeysFromComplexAnimator();
    void prp_addAllKeysToComplexAnimator();
    bool prp_hasKeys();

    void anim_setRecordingWithoutChangingKeys(bool rec,
                                              bool saveUndoRedo = true);
    bool prp_isRecording();
    virtual void anim_removeAllKeys();
    bool prp_isKeyOnCurrentFrame();
    virtual void prp_getKeysInRect(QRectF selectionRect,
                                   qreal pixelsPerFrame,
                                   QList<Key *> *keysList);
    bool anim_getNextAndPreviousKeyIdForRelFrame(
                                 int *prevIdP,
                                 int *nextIdP,
                                 int frame) const;
    virtual void prp_drawKeys(QPainter *p,
                              qreal pixelsPerFrame, qreal drawY,
                              int startFrame, int endFrame);
    Key *anim_getKeyAtAbsFrame(const int &frame);

    virtual void anim_saveCurrentValueAsKey() {}
protected:
    virtual void anim_drawKey(QPainter *p,
                         Key *key,
                         const qreal &pixelsPerFrame,
                         const qreal &drawY,
                         const int &startFrame);
    bool anim_mTraceKeyOnCurrentFrame = false;
    QList<Key*> anim_mKeys;
    int anim_mCurrentAbsFrame = 0;
    int anim_mCurrentRelFrame = 0;
    bool anim_mIsComplexAnimator = false;
    bool anim_mIsCurrentAnimator = false;
    bool anim_mIsRecording = false;
    bool anim_mKeyOnCurrentFrame = false;
    QColor anim_mAnimatorColor;
public slots:
    void anim_deleteCurrentKey();
    void anim_updateAfterShifted();
signals:
    void beingDeleted();
};

#endif // ANIMATOR_H
