#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QColor>
#include <QPainterPath>

class QIODevice;
#include "Properties/property.h"

class ComplexAnimator;
class Key;
class QrealPoint;
class QPainter;
class DurationRectangleMovable;
class FakeComplexAnimator;
enum CtrlsMode : short;

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

    virtual void anim_updateAfterChangedKey(Key * const key);

    virtual void prp_setAbsFrame(const int &frame);

    virtual void prp_switchRecording();

    virtual bool prp_isDescendantRecording() const;

    bool anim_isComplexAnimator();

    virtual void prp_startDragging();

    void anim_updateRelFrame();
    virtual void anim_mergeKeysIfNeeded();
    bool anim_getClosestsKeyOccupiedRelFrame(const int &frame,
                                             int &closest);
    Key *anim_getKeyAtRelFrame(const int &frame) const;
    bool anim_hasPrevKey(const Key * const key);
    bool anim_hasNextKey(const Key * const key);
    virtual void anim_callFrameChangeUpdater();
    void anim_sortKeys();

    virtual void anim_appendKey(const stdsptr<Key> &newKey);
    virtual void anim_removeKey(const stdsptr<Key>& keyToRemove);
    virtual void anim_moveKeyToRelFrame(Key *key, const int &newFrame);

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
    bool prp_hasKeys() const;

    void anim_setRecordingWithoutChangingKeys(const bool &rec);
    bool prp_isRecording();
    virtual void anim_removeAllKeys();
    bool prp_isKeyOnCurrentFrame() const;
    void prp_getKeysInRect(const QRectF &selectionRect,
                           const qreal &pixelsPerFrame,
                           QList<Key*>& keysList,
                           const int &keyRectSize);
    bool anim_getNextAndPreviousKeyIdForRelFrame(
            int &prevIdP, int &nextIdP, const int &frame) const;
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

    Key *anim_getNextKey(const Key * const key) const;
    Key* anim_getPrevKey(const Key * const key) const;
    int anim_getNextKeyRelFrame(const Key * const key) const;
    int anim_getPrevKeyRelFrame(const Key * const key) const;
    void anim_setRecordingValue(const bool &rec);

    bool SWT_isAnimator() const;
    int anim_getCurrentAbsFrame();
    int anim_getCurrentRelFrame();
    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;
    std::pair<Key*, Key*> anim_getPrevAndNextKey(const int &relFrame) const;
    Key* anim_getPrevKey(const int &relFrame) const;
    Key* anim_getNextKey(const int &relFrame) const;
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
    int anim_getPrevKeyRelFrame(const int &relFrame) const;
    int anim_getNextKeyRelFrame(const int &relFrame) const;
    bool anim_getNextAndPreviousKeyIdForRelFrameF(int &prevIdP, int &nextIdP,
                                                  const qreal &frame) const;
    bool hasSelectedKeys() const;

    void addKeyToSelected(Key* key);
    void removeKeyFromSelected(Key* key);

    void writeSelectedKeys(QIODevice* target);

    void deselectAllKeys();
    void selectAllKeys();

    void incSelectedKeysFrame(const int& dFrame);

    void scaleSelectedKeysFrame(const int& absPivotFrame,
                                const qreal& scale);

    void cancelSelectedKeysTransform();
    void finishSelectedKeysTransform();
    void startSelectedKeysTransform();

    void deleteSelectedKeys();

    int getLowestAbsFrameForSelectedKey();

    const QList<stdptr<Key>>& getSelectedKeys() const {
        return anim_mSelectedKeys;
    }
    void prp_updateAfterChangedAbsFrameRange(const FrameRange &range);
    int getInsertIdForKeyRelFrame(const int &relFrame) const;
private:
    void sortSelectedKeys();
protected:
    Animator(const QString &name);


    int anim_getKeyIndex(const Key * const key) const;

    virtual void anim_drawKey(QPainter *p, Key* key,
                              const qreal &pixelsPerFrame,
                              const qreal &drawY,
                              const int &startFrame,
                              const int& rowHeight,
                              const int& keyRectSize);
    bool anim_mIsComplexAnimator = false;
    bool anim_mIsRecording = false;

    int anim_mCurrentAbsFrame = 0;
    int anim_mCurrentRelFrame = 0;

    QList<stdsptr<Key>> anim_mKeys;
    QList<stdptr<Key>> anim_mSelectedKeys;
    stdptr<Key> anim_mKeyOnCurrentFrame;
    qsptr<FakeComplexAnimator> mFakeComplexAnimator;
public slots:
    void anim_deleteCurrentKey();
    virtual void anim_updateAfterShifted();
    void prp_setRecording(const bool &rec);

private:
    int getInsertIdForKeyRelFrame(const int &relFrame,
                                  const int &min, const int &max) const;
};

#endif // ANIMATOR_H
