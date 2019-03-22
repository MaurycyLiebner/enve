#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QColor>
#include <QPainterPath>

class QIODevice;
#include "Properties/property.h"
#include "key.h"

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

    void prp_startDragging();
    void prp_updateAfterChangedAbsFrameRange(const FrameRange &range);
    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;
    virtual void anim_scaleTime(const int &pivotAbsFrame,
                                const qreal &scale);

    virtual bool anim_prevRelFrameWithKey(const int &relFrame,
                                          int &prevRelFrame);
    virtual bool anim_nextRelFrameWithKey(const int &relFrame,
                                          int &nextRelFrame);
    virtual stdsptr<Key> readKey(QIODevice *target);
    virtual Key *anim_getKeyAtPos(const qreal &relX,
                                 const int &minViewedFrame,
                                 const qreal &pixelsPerFrame,
                                 const int &keyRectSize);
    virtual void anim_removeAllKeysFromComplexAnimator(ComplexAnimator *target);
    virtual void anim_updateAfterChangedKey(Key * const key);
    virtual void anim_setAbsFrame(const int &frame);
    virtual bool anim_isDescendantRecording() const;
    virtual void anim_mergeKeysIfNeeded();
    virtual void anim_appendKey(const stdsptr<Key> &newKey);
    virtual void anim_removeKey(const stdsptr<Key>& keyToRemove);
    virtual void anim_moveKeyToRelFrame(Key *key, const int &newFrame);
    virtual void anim_keyValueChanged(Key *key);

    virtual DurationRectangleMovable *anim_getRectangleMovableAtPos(
                                           const int &relX,
                                           const int &minViewedFrame,
                                           const qreal &pixelsPerFrame);
    virtual bool anim_hasKeys() const;
    virtual bool anim_isRecording();
    virtual void anim_removeAllKeys();
    virtual void anim_getKeysInRect(const QRectF &selectionRect,
                                    const qreal &pixelsPerFrame,
                                    QList<Key*>& keysList,
                                    const int &keyRectSize);
    virtual void anim_drawKeys(QPainter *p,
                               const qreal &pixelsPerFrame,
                               const qreal &drawY,
                               const int &startFrame,
                               const int &endFrame,
                               const int &rowHeight,
                               const int &keyRectSize);

    virtual void anim_saveCurrentValueAsKey();
    virtual void anim_addKeyAtRelFrame(const int &relFrame);
    bool SWT_isAnimator() const;
protected:
    virtual void anim_afterKeyOnCurrentFrameChanged(Key* const key) {
        Q_UNUSED(key);
    }
    void anim_drawKey(QPainter * const p, Key * const key,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame,
                      const int& rowHeight,
                      const int& keyRectSize);
public slots:
    virtual void anim_updateAfterShifted();
    virtual void anim_setRecording(const bool &rec);
public:
    void anim_updateRelFrame();
    void anim_switchRecording();

    bool anim_getClosestsKeyOccupiedRelFrame(const int &frame,
                                             int &closest);
    void anim_updateKeyOnCurrrentFrame();
    template <class T = Key>
    T* anim_getKeyOnCurrentFrame() const;
    template <class T = Key>
    T *anim_getKeyAtRelFrame(const int &frame) const;
    template <class T = Key>
    T *anim_getKeyAtAbsFrame(const int &frame) const;
    template <class T = Key>
    T *anim_getNextKey(const Key * const key) const;
    template <class T = Key>
    T *anim_getPrevKey(const Key * const key) const;
    template <class T = Key>
    std::pair<T*, T*> anim_getPrevAndNextKey(const int &relFrame) const;
    template <class T = Key>
    T* anim_getPrevKey(const int &relFrame) const;
    template <class T = Key>
    T* anim_getNextKey(const int &relFrame) const;

    int anim_getPrevKeyId(const int &relFrame) const;
    int anim_getNextKeyId(const int &relFrame) const;

    int anim_getNextKeyRelFrame(const Key * const key) const;
    int anim_getPrevKeyRelFrame(const Key * const key) const;

    bool anim_hasPrevKey(const Key * const key);
    bool anim_hasNextKey(const Key * const key);

    void anim_callFrameChangeUpdater();
    void anim_sortKeys();

    void anim_addAllKeysToComplexAnimator(ComplexAnimator *target);

    void anim_setRecordingWithoutChangingKeys(const bool &rec);

    bool anim_getNextAndPreviousKeyIdForRelFrame(
            int &prevIdP, int &nextIdP, const int &frame) const;


    void anim_setRecordingValue(const bool &rec);

    int anim_getCurrentAbsFrame();
    int anim_getCurrentRelFrame();

    void anim_shiftAllKeys(const int &shift);

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
    int getInsertIdForKeyRelFrame(const int &relFrame) const;
private:
    void sortSelectedKeys();
protected:
    Animator(const QString &name);

    template <class T = Key>
    T* anim_getKeyAtIndex(const int& id) const;
    int anim_getKeyIndex(const Key * const key) const;

    bool anim_mIsRecording = false;

    int anim_mCurrentAbsFrame = 0;
    int anim_mCurrentRelFrame = 0;

    QList<stdsptr<Key>> anim_mKeys;
    QList<stdptr<Key>> anim_mSelectedKeys;
    qsptr<FakeComplexAnimator> mFakeComplexAnimator;
public slots:
    void anim_deleteCurrentKey();
signals:
    void anim_isRecordingChanged();
private:
    void anim_setKeyOnCurrentFrame(Key * const key);
    int getInsertIdForKeyRelFrame(const int &relFrame,
                                  const int &min, const int &max) const;
    void anim_afterInsertedKey(const int &insertId,
                               Key * const insertedKey) const;
    void anim_afterRemovedKey(const int &removedId) const;

    stdptr<Key> anim_mKeyOnCurrentFrame;
};


template <class T>
T *Animator::anim_getKeyOnCurrentFrame() const {
    return static_cast<T*>(anim_mKeyOnCurrentFrame.data());
}

template <class T>
T* Animator::anim_getKeyAtIndex(const int& id) const {
    if(id < 0 || id >= anim_mKeys.count()) return nullptr;
    return static_cast<T*>(anim_mKeys.at(id).get());
}

template <class T>
T *Animator::anim_getKeyAtRelFrame(const int &frame) const {
    if(anim_mKeys.isEmpty()) return nullptr;
    if(frame > anim_mKeys.last()->getRelFrame()) return nullptr;
    if(frame < anim_mKeys.first()->getRelFrame()) return nullptr;
    int minId = 0;
    int maxId = anim_mKeys.count() - 1;
    while(maxId - minId > 1) {
        const int guess = (maxId + minId)/2;
        if(anim_mKeys.at(guess)->getRelFrame() > frame) {
            maxId = guess;
        } else {
            minId = guess;
        }
    }
    T * const minIdKey = anim_getKeyAtIndex<T>(minId);
    if(minIdKey->getRelFrame() == frame) return minIdKey;
    T * const maxIdKey = anim_getKeyAtIndex<T>(maxId);
    if(maxIdKey->getRelFrame() == frame) return maxIdKey;
    return nullptr;
}

template <class T>
T *Animator::anim_getKeyAtAbsFrame(const int &frame) const {
    return anim_getKeyAtRelFrame<T>(prp_absFrameToRelFrame(frame));
}

template <class T>
T* Animator::anim_getPrevKey(const Key * const key) const {
    const int keyId = anim_getKeyIndex(key);
    if(keyId <= 0) return nullptr;
    return static_cast<T*>(anim_mKeys.at(keyId - 1).get());
}

template <class T>
T* Animator::anim_getNextKey(const Key * const key) const {
    const int keyId = anim_getKeyIndex(key);
    if(keyId == anim_mKeys.count() - 1) return nullptr;
    return anim_mKeys.at(keyId + 1).get();
}

template <class T>
std::pair<T*, T*> Animator::anim_getPrevAndNextKey(const int &relFrame) const {
    std::pair<T*, T*> result(nullptr, nullptr);
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame)) {
        T * const prevKey = anim_getKeyAtIndex<T>(prevId);
        if(prevKey->getRelFrame() < relFrame) {
            result.first = prevKey;
        } else if(prevKey->getRelFrame() == relFrame) {
            if(prevId > 0) {
                result.first = anim_getKeyAtIndex<T>(prevId - 1);
            }
        }
        T * const nextKey = anim_getKeyAtIndex<T>(nextId);
        if(nextKey->getRelFrame() > relFrame) {
            result.second = prevKey;
        } else if(nextKey->getRelFrame() == relFrame) {
            if(nextId + 1 < anim_mKeys.count()) {
                result.second = anim_getKeyAtIndex<T>(nextId + 1);
            }
        }
    }
    return result;
}

int Animator::anim_getPrevKeyId(const int &relFrame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame)) {
        Key * const key = anim_getKeyAtIndex(prevId);
        if(key->getRelFrame() < relFrame) {
            return prevId;
        } else if(key->getRelFrame() == relFrame) {
            if(prevId > 0) {
                return prevId - 1;
            }
        }
    }
    return -1;
}

int Animator::anim_getNextKeyId(const int &relFrame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame)) {
        Key * const key = anim_getKeyAtIndex(nextId);
        if(key->getRelFrame() > relFrame) {
            return nextId;
        } else if(key->getRelFrame() == relFrame) {
            if(nextId + 1 < anim_mKeys.count()) {
                return nextId + 1;
            }
        }
    }
    return -1;
}

template <class T>
T *Animator::anim_getPrevKey(const int &relFrame) const {
    return anim_getKeyAtIndex<T>(anim_getPrevKeyId(relFrame));
}

template <class T>
T *Animator::anim_getNextKey(const int &relFrame) const {
    return anim_getKeyAtIndex<T>(anim_getNextKeyId(relFrame));
}


#endif // ANIMATOR_H
