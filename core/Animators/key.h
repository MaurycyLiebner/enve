#ifndef KEY_H
#define KEY_H
#include "smartPointers/sharedpointerdefs.h"
#include "pointhelpers.h"
#include "Animators/animator.h"

#include <QtCore>
class QPainter;
class KeyCloner;

class KeysClipboardContainer;

class Key : public StdSelfRef {
    friend class Animator;
    friend class KeysClipboardContainer;
public:
    Key(Animator * const parentAnimator);
    Key(const int &frame, Animator * const parentAnimator);

//    QrealPoint *mousePress(qreal frameT, qreal valueT,
//                    qreal pixelsPerFrame, qreal pixelsPerValue);
    virtual ~Key() {}

    virtual void startFrameTransform();
    virtual void finishFrameTransform();
    virtual void mergeWith(const stdsptr<Key>& key) { key->removeFromAnimator(); }

    virtual void deleteKey() {
        removeFromAnimator();
    }

    virtual bool differsFromKey(Key* key) const = 0;
    virtual void writeKey(QIODevice *target);
    virtual void readKey(QIODevice *target);


    virtual void cancelFrameTransform();
    virtual void scaleFrameAndUpdateParentAnimator(
            const int &relativeToFrame,
            const qreal &scaleFactor,
            const bool& useSavedFrame);

    virtual bool isSelected() const;
    virtual void addToSelection(QList<qptr<Animator>> &selectedAnimators);
    virtual void removeFromSelection(QList<qptr<Animator>> &selectedAnimators);
    virtual bool isDescendantSelected() const { return isSelected(); }


    int getAbsFrame() const;

    bool hasPrevKey() const;
    bool hasNextKey() const;
    void incFrameAndUpdateParentAnimator(const int &inc);
    void setRelFrameAndUpdateParentAnimator(const int &relFrame);
    template <class T = Animator>
    T* getParentAnimator() const {
        return static_cast<T*>(mParentAnimator.data());
    }
    void removeFromAnimator();

    void setSelected(const bool &bT);

    bool isHovered() const {
        return mHovered;
    }

    void setHovered(const bool &bT) {
        mHovered = bT;
    }

    int getRelFrame() const;
    void setAbsFrame(const int &frame);

    template <class T = Key>
    T* getNextKey() const {
        return static_cast<T*>(mParentAnimator->anim_getNextKey(this));
    }

    template <class T = Key>
    T* getPrevKey() const {
        return static_cast<T*>(mParentAnimator->anim_getPrevKey(this));
    }

    bool differesFromNextKey() const {
        return differsFromKey(getNextKey());
    }

    bool differesFromPrevKey() const {
        return differsFromKey(getPrevKey());
    }

    void afterKeyChanged();

    int getPrevKeyRelFrame() const {
        auto prevKey = getPrevKey();
        if(!prevKey) return mRelFrame;
        return prevKey->getRelFrame();
    }

    int getNextKeyRelFrame() const {
        auto nextKey = getNextKey();
        if(!nextKey) return mRelFrame;
        return getNextKey()->getRelFrame();
    }

    virtual void  updateAfterPrevKeyChanged(Key * const prevKey) {
        Q_UNUSED(prevKey);
    }

    virtual void  updateAfterNextKeyChanged(Key * const nextKey) {
        Q_UNUSED(nextKey);
    }

    void updateAfterNeighbouringKeysChanged(Key * const prevKey,
                                            Key * const nextKey) {
        updateAfterPrevKeyChanged(prevKey);
        updateAfterNextKeyChanged(nextKey);
    }
protected:
    virtual void setRelFrame(const int &frame);
    bool mIsSelected = false;
    bool mHovered = false;

    int mRelFrame;
    int mSavedRelFrame;

    QPointer<Animator> mParentAnimator;
};

struct KeyPair {
    KeyPair(Key* key1T, Key* key2T) {
        key1 = key1T;
        key2 = key2T;
    }

    void merge() const {
        key1->mergeWith(GetAsSPtr(key2, Key));
    }

    Key* key1;
    Key* key2;
};

#endif // KEY_H
