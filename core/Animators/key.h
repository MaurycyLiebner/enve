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
public:
    Key(Animator * const parentAnimator);
//    QrealPoint *mousePress(qreal frameT, qreal valueT,
//                    qreal pixelsPerFrame, qreal pixelsPerValue);
    virtual ~Key() {}

    virtual void startFrameTransform();
    virtual void finishFrameTransform();

    int getAbsFrame() const;
    virtual void setRelFrame(const int &frame);

    bool hasPrevKey() const;
    bool hasNextKey() const;
    void incFrameAndUpdateParentAnimator(const int &inc,
                                         const bool &finish = true);
    void setRelFrameAndUpdateParentAnimator(const int &relFrame,
                                         const bool &finish = true);
    template <class T = Animator>
    T* getParentAnimator() const {
        return static_cast<T*>(mParentAnimator.data());
    }

    virtual void mergeWith(const stdsptr<Key>& key) { key->removeFromAnimator(); }

    virtual bool isDescendantSelected() const { return isSelected(); }

    void removeFromAnimator();

    virtual void deleteKey() {
        removeFromAnimator();
    }

    virtual void cancelFrameTransform();
    virtual void scaleFrameAndUpdateParentAnimator(
            const int &relativeToFrame,
            const qreal &scaleFactor,
            const bool &useSavedFrame = true);
    void setSelected(const bool &bT);
    virtual bool isSelected() const;

    virtual void addToSelection(QList<qptr<Animator>> &selectedAnimators);
    virtual void removeFromSelection(QList<qptr<Animator>> &selectedAnimators);

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

    virtual bool differsFromKey(Key* key) const = 0;
    virtual void writeKey(QIODevice *target);
    virtual void readKey(QIODevice *target);

    void afterKeyChanged();

    virtual void saveCurrentFrameAndValue() {
        mSavedRelFrame = mRelFrame;
    }


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
protected:
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
