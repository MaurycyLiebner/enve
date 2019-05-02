#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QColor>
#include <QPainterPath>

class QIODevice;
#include "Properties/property.h"
#include "key.h"

class ComplexAnimator;
class Key;
class ComplexKey;
class QrealPoint;
class QPainter;
class DurationRectangleMovable;
class FakeComplexAnimator;
enum CtrlsMode : short;

class Animator : public Property {
    Q_OBJECT
    friend class OverlappingKeys;
protected:
    Animator(const QString &name);

    virtual void anim_afterKeyOnCurrentFrameChanged(Key* const key) {
        Q_UNUSED(key);
    }
public:
    class OverlappingKeys {
    public:
        OverlappingKeys(const stdsptr<Key>& key,
                        Animator * const animator) :
            mAnimator(animator) {
            mKeys << key;
        }

        int getFrame() const {
            const auto key = getKey();
            if(key) return key->getRelFrame();
            return 0;
        }

        Key * getKey() const {
            if(mKeys.isEmpty()) return nullptr;
            for(const auto& iKey : mKeys) {
                if(iKey->isDescendantSelected()) return iKey.get();
            }
            return mKeys.last().get();
        }

        bool isEmpty() const {
            return mKeys.isEmpty();
        }

        void removeKey(const stdsptr<Key>& key) {
            for(int i = 0; i < mKeys.count(); i++) {
                const auto& iKey = mKeys.at(i);
                if(iKey.get() == key.get()) {
                    mKeys.removeAt(i);
                    break;
                }
            }
        }

        void addKey(const stdsptr<Key>& key) {
            mKeys.append(key);
        }

        void merge();
    private:
        Animator * mAnimator;
        QList<stdsptr<Key>> mKeys;
    };

    class OverlappingKeyList {
    public:
        OverlappingKeyList(Animator * const animator) :
            mAnimator(animator) {}

        class iterator {
        public:
            iterator(const int& id,
                    const QList<OverlappingKeys> * const list) :
                mList(list) {
                setId(id);
            }

            iterator& operator++() {
                setId(mId + 1);
                return *this;
            }

            bool operator!=(const iterator& other) const {
                return this->mId != other.mId;
            }

            Key * const & operator->() const { return mKey; }
            Key * const &operator*() const { return mKey; }
        protected:
            int mId;
        private:
            void setId(const int& id) {
                mId = id;
                if(mId < 0 || mId >= mList->count()) mKey = nullptr;
                else mKey = mList->at(mId).getKey();
            }
            const QList<OverlappingKeys> * mList = nullptr;
            Key * mKey = nullptr;
        };

        iterator begin() const {
            return iterator(0, &mList);
        }

        iterator end() const {
            return iterator(mList.count(), &mList);
        }

        int count() const {
            return mList.count();
        }

        bool isEmpty() const {
            return mList.isEmpty();
        }

        int add(const stdsptr<Key>& key) {
            const int relFrame = key->getRelFrame();
            const int insertId = idForRelFrame(relFrame);
            if(insertId < 0 || insertId >= mList.count()) {
                mList.insert(insertId, OverlappingKeys(key, mAnimator));
            } else {
                auto& ovrlp = mList[insertId];
                if(ovrlp.getFrame() == relFrame) ovrlp.addKey(key);
                else mList.insert(insertId, OverlappingKeys(key, mAnimator));
            }
            return insertId;
        }

        void remove(const stdsptr<Key>& key) {
            const int relFrame = key->getRelFrame();
            const int removeId = idForRelFrame(relFrame);
            auto& ovrlp = mList[removeId];
            if(ovrlp.getFrame() == relFrame) {
                ovrlp.removeKey(key);
                if(ovrlp.isEmpty()) mList.removeAt(removeId);
            }
        }

//        void removeAt(const int& id) {
//            if(id < 0) return;
//            if(id >= mList.count()) return;
//            return mList.removeAt(id);
//        }

//        void removeAtRelFrame(const int& relFrame) {
//            const int id = getIdForRelFrame(relFrame);
//            const auto kAtId = atId(id);
//            if(!kAtId) return;
//            if(kAtId->getRelFrame() != relFrame) return;
//            return mList.removeAt(id);
//        }

        template <class T = Key>
        T * atId(const int& id) const {
            if(id < 0) return nullptr;
            if(id >= mList.count()) return nullptr;
            return static_cast<T*>(mList.at(id).getKey());
        }

        template <class T = Key>
        T * atRelFrame(const int& relFrame) const {
            const int id = idForRelFrame(relFrame);
            const auto kAtId = atId<T>(id);
            if(!kAtId) return nullptr;
            if(kAtId->getRelFrame() == relFrame) return kAtId;
            return nullptr;
        }

        template <class T = Key>
        T * first() const {
            return atId<T>(0);
        }

        template <class T = Key>
        T * last() const {
            return atId<T>(mList.count() - 1);
        }

        int idForRelFrame(const int& relFrame) const {
            return idForRelFrame(relFrame, 0, mList.count());
        }

        void mergeAll() {
            for(int i = 0; i < mList.count(); i++) {
                auto& iOKey = mList[i];
                iOKey.merge();
            }
        }
    private:
        int idForRelFrame(const int& relFrame,
                          const int& min, const int& max) const {
            if(min >= max) return min;
            const int guess = (max + min)/2;
            const Key * const key = mList.at(guess).getKey();
            const int guessFrame = key->getRelFrame();
            if(guessFrame > relFrame) {
                return idForRelFrame(relFrame, min, guess);
            } else if(guessFrame < relFrame) {
                return idForRelFrame(relFrame, guess + 1, max);
            }
            // guessFrame == relFrame
            return guess;
        }

        Animator * const mAnimator;
        QList<OverlappingKeys> mList;
    };

    enum UpdateReason {
        FRAME_CHANGE,
        CHILD_USER_CHANGE,
        USER_CHANGE
    };

    virtual void anim_addKeyAtRelFrame(const int &relFrame) = 0;
    virtual void anim_saveCurrentValueAsKey() = 0;
    virtual stdsptr<Key> readKey(QIODevice *target) = 0;

    virtual void anim_scaleTime(const int &pivotAbsFrame,
                                const qreal &scale);

    virtual bool anim_prevRelFrameWithKey(const int &relFrame,
                                          int &prevRelFrame);
    virtual bool anim_nextRelFrameWithKey(const int &relFrame,
                                          int &nextRelFrame);
    virtual Key *anim_getKeyAtPos(const qreal &relX,
                                 const int &minViewedFrame,
                                 const qreal &pixelsPerFrame,
                                 const int &keyRectSize);
    virtual void anim_removeAllKeysFromComplexAnimator(ComplexAnimator *target);
    virtual void anim_setAbsFrame(const int &frame);
    virtual bool anim_isDescendantRecording() const;

    virtual void anim_appendKey(const stdsptr<Key> &newKey);
    virtual void anim_removeKey(const stdsptr<Key>& keyToRemove);

    virtual DurationRectangleMovable *anim_getRectangleMovableAtPos(
                                           const int &relX,
                                           const int &minViewedFrame,
                                           const qreal &pixelsPerFrame);
    virtual void anim_removeAllKeys();
    virtual void anim_getKeysInRect(const QRectF &selectionRect,
                                    const qreal &pixelsPerFrame,
                                    QList<Key*>& keysList,
                                    const int &keyRectSize);
    virtual void anim_updateAfterShifted();
    virtual void anim_setRecording(const bool &rec);

    void drawTimelineControls(QPainter * const p,
                              const qreal &pixelsPerFrame,
                              const FrameRange &absFrameRange,
                              const int &rowHeight);

    void addActionsToMenu(PropertyTypeMenu * const menu);

    bool SWT_isAnimator() const;
    void prp_startDragging();
    void prp_updateAfterChangedAbsFrameRange(const FrameRange &range);
    FrameRange prp_getIdenticalRelRange(const int &relFrame) const;
public:
    void anim_mergeKeysIfNeeded();
    void anim_updateAfterChangedKey(Key * const key);

    bool anim_hasKeys() const;
    bool anim_isRecording();

    void anim_updateRelFrame();
    void anim_switchRecording();

    bool anim_getClosestsKeyOccupiedRelFrame(const int &frame,
                                             int &closest);
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

    void anim_addAllKeysToComplexAnimator(ComplexAnimator *target);

    void anim_setRecordingWithoutChangingKeys(const bool &rec);

    std::pair<int, int> anim_getPrevAndNextKeyId(const int &relFrame) const;
    std::pair<int, int> anim_getPrevAndNextKeyIdF(const qreal &relFrame) const;

    void anim_setRecordingValue(const bool &rec);

    int anim_getCurrentRelFrame() const;
    int anim_getCurrentAbsFrame() const;

    void anim_moveKeyToRelFrame(Key *key, const int &newFrame);
    void anim_shiftAllKeys(const int &shift);

    bool hasFakeComplexAnimator();

    FakeComplexAnimator *getFakeComplexAnimator();

    void enableFakeComplexAnimator();

    void disableFakeComplexAnimator();
    void disableFakeComplexAnimatrIfNotNeeded();
    int anim_getPrevKeyRelFrame(const int &relFrame) const;
    int anim_getNextKeyRelFrame(const int &relFrame) const;
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
    void anim_deleteCurrentKey();

    int getLowestAbsFrameForSelectedKey();

    const QList<stdptr<Key>>& getSelectedKeys() const {
        return anim_mSelectedKeys;
    }
    int getInsertIdForKeyRelFrame(const int &relFrame) const;

    template <class T = Key>
    T* anim_getKeyAtIndex(const int& id) const;
    int anim_getKeyIndex(const Key * const key) const;

    void anim_coordinateKeysWith(Animator * const other);
    void anim_addKeysWhereOtherHasKeys(const Animator * const other);
protected:
    void readKeys(QIODevice *target);
    void writeKeys(QIODevice *target) const;

    IdRange frameRangeToKeyIdRange(const FrameRange& relRange) const {
        int min = anim_getPrevKeyId(relRange.fMin + 1);
        int max = anim_getNextKeyId(relRange.fMax - 1);
        if(min == -1) min = 0;
        if(max == -1) max = anim_mKeys.count() - 1;
        return {min, max};
    }

    OverlappingKeyList anim_mKeys;
    QList<stdptr<Key>> anim_mSelectedKeys;
    qsptr<FakeComplexAnimator> mFakeComplexAnimator;
private:
    void anim_drawKey(QPainter * const p, Key * const key,
                      const qreal &pixelsPerFrame,
                      const int &startFrame,
                      const int& rowHeight);

    bool anim_mIsRecording = false;

    int anim_mCurrentAbsFrame = 0;
    int anim_mCurrentRelFrame = 0;
signals:
    void anim_isRecordingChanged();
private:
    void anim_updateKeyOnCurrrentFrame();
    void anim_setKeyOnCurrentFrame(Key * const key);
    int getInsertIdForKeyRelFrame(const int &relFrame,
                                  const int &min, const int &max) const;
    stdptr<Key> anim_mKeyOnCurrentFrame;
};


template <class T>
T *Animator::anim_getKeyOnCurrentFrame() const {
    return static_cast<T*>(anim_mKeyOnCurrentFrame.data());
}

template <class T>
T* Animator::anim_getKeyAtIndex(const int& id) const {
    if(id < 0 || id >= anim_mKeys.count()) return nullptr;
    return anim_mKeys.atId<T>(id);
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
        if(anim_mKeys.atId(guess)->getRelFrame() > frame) {
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
    return anim_mKeys.atId<T>(keyId - 1);
}

template <class T>
T* Animator::anim_getNextKey(const Key * const key) const {
    const int keyId = anim_getKeyIndex(key);
    if(keyId == anim_mKeys.count() - 1) return nullptr;
    return anim_mKeys.atId<T>(keyId + 1);
}

template <class T>
std::pair<T*, T*> Animator::anim_getPrevAndNextKey(const int &relFrame) const {
    const auto prevNext = anim_getPrevAndNextKeyId(relFrame);
    const int prevId = prevNext.first;
    T * const prevKey = anim_getKeyAtIndex<T>(prevId);
    const int nextId = prevNext.second;
    T * const nextKey = anim_getKeyAtIndex<T>(nextId);
    return {prevKey, nextKey};
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
