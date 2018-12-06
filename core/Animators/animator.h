#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QColor>
class QIODevice;
#include "Properties/property.h"

class ComplexAnimator;
class Key;
class QPainter;
class DurationRectangleMovable;
class FakeComplexAnimator;

class Animator : public Property {
    Q_OBJECT
public:
    enum UpdateReason {
        FRAME_CHANGE,
        USER_CHANGE,
        CHILD_USER_CHANGE,
        PARENT_USER_CHANGE
    };

    virtual void scaleTime(const int &pivotAbsFrame, const qreal &scale);

    virtual void anim_updateAfterChangedKey(Key *key);

    virtual void prp_setAbsFrame(const int &frame);

    virtual void prp_switchRecording();

    virtual bool prp_isDescendantRecording();

    bool anim_isComplexAnimator();

    virtual bool prp_isAnimator();
    virtual void prp_startDragging();

    void anim_updateRelFrame();
    virtual void anim_mergeKeysIfNeeded();
    bool anim_getClosestsKeyOccupiedRelFrame(const int &frame,
                                             int &closest);
    Key *anim_getKeyAtRelFrame(const int &frame);
    bool anim_hasPrevKey(Key *key);
    bool anim_hasNextKey(Key *key);
    virtual void anim_callFrameChangeUpdater();
    virtual void anim_sortKeys();

    virtual void anim_appendKey(const stdsptr<Key> &newKey,
                                const bool &saveUndoRedo = true,
                                const bool &update = true);
    virtual void anim_removeKey(const stdsptr<Key>& keyToRemove,
                                const bool &saveUndoRedo = true);
    virtual void anim_moveKeyToRelFrame(Key *key,
                                        const int &newFrame,
                                        const bool &saveUndoRedo = true,
                                        const bool &finish = true);

    virtual void anim_keyValueChanged(Key *key);

    void anim_updateKeyOnCurrrentFrame();

    virtual DurationRectangleMovable *anim_getRectangleMovableAtPos(
                                           const int &relX,
                                           const int &minViewedFrame,
                                           const qreal &pixelsPerFrame);

    Key *prp_getKeyAtPos(const qreal &relX,
                         const int &minViewedFrame,
                         const qreal &pixelsPerFrame,
                         const int &keyRectSize);
    void prp_removeAllKeysFromComplexAnimator(ComplexAnimator *target);
    void prp_addAllKeysToComplexAnimator(ComplexAnimator *target);
    bool prp_hasKeys();

    void anim_setRecordingWithoutChangingKeys(const bool &rec,
                                              const bool &saveUndoRedo = true);
    bool prp_isRecording();
    virtual void anim_removeAllKeys();
    bool prp_isKeyOnCurrentFrame();
    void prp_getKeysInRect(const QRectF &selectionRect,
                           const qreal &pixelsPerFrame,
                           QList<Key*>& keysList,
                           const int &keyRectSize);
    bool anim_getNextAndPreviousKeyIdForRelFrame(
                                 int *prevIdP,
                                 int *nextIdP,
                                 const int &frame) const;
    virtual void prp_drawKeys(QPainter *p,
                              const qreal &pixelsPerFrame,
                              const qreal &drawY,
                              const int &startFrame,
                              const int &endFrame,
                              const int &rowHeight,
                              const int &keyRectSize);
    Key *anim_getKeyAtAbsFrame(const int &frame);

    virtual void anim_saveCurrentValueAsKey();
    virtual void anim_addKeyAtRelFrame(const int &relFrame);

    Key *anim_getNextKey(Key* key);
    Key* anim_getPrevKey(Key* key);
    int anim_getNextKeyRelFrame(Key* key);
    int anim_getPrevKeyRelFrame(Key* key);
    void anim_setRecordingValue(const bool &rec);

    bool SWT_isAnimator();
    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    int anim_getCurrentAbsFrame();
    int anim_getCurrentRelFrame();
    virtual void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                                      int *lastIdentical,
                                                      const int &relFrame);
    Key* anim_getPrevKey(const int &relFrame);
    Key* anim_getNextKey(const int &relFrame);
    void anim_shiftAllKeys(const int &shift);

    bool prp_nextRelFrameWithKey(const int &relFrame,
                                 int &nextRelFrame);
    bool prp_prevRelFrameWithKey(const int &relFrame,
                                 int &prevRelFrame);
    virtual stdsptr<Key> readKey(QIODevice *target);

    bool hasFakeComplexAnimator();

    FakeComplexAnimator *getFakeComplexAnimator();

    void enableFakeComplexAnimator();

    void disableFakeComplexAnimator();
    void disableFakeComplexAnimatrIfNotNeeded();
    int anim_getPrevKeyRelFrame(const int &relFrame);
    int anim_getNextKeyRelFrame(const int &relFrame);
    bool anim_getNextAndPreviousKeyIdForRelFrameF(int *prevIdP,
                                                  int *nextIdP,
                                                  const qreal &frame) const;
    bool hasSelectedKeys() const;

    void addSelectedKey(Key* key);

    void removeSelectedKey(Key* key);

    void writeSelectedKeys(QIODevice* target);
protected:
    Animator(const QString &name);

    int anim_getKeyIndex(Key *key);

    virtual void anim_drawKey(QPainter *p, Key* key,
                              const qreal &pixelsPerFrame,
                              const qreal &drawY,
                              const int &startFrame,
                              const int& rowHeight,
                              const int& keyRectSize);
    bool anim_mIsComplexAnimator = false;
    bool anim_mIsCurrentAnimator = false;
    bool anim_mIsRecording = false;

    int anim_mCurrentAbsFrame = 0;
    int anim_mCurrentRelFrame = 0;

    QColor anim_mAnimatorColor;

    QList<stdsptr<Key>> anim_mKeys;
    QList<stdptr<Key>> anim_mSelectedKeys;
    stdptr<Key> anim_mKeyOnCurrentFrame;
    qsptr<FakeComplexAnimator> mFakeComplexAnimator;
public slots:
    void anim_deleteCurrentKey();
    virtual void anim_updateAfterShifted();
    void prp_setRecording(const bool &rec);
};

#endif // ANIMATOR_H
