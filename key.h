#ifndef KEY_H
#define KEY_H
#include "selfref.h"
#include "sharedpointerdefs.h"
#include <QtCore>

class ComplexAnimator;

class ComplexKey;
class KeyCloner;

class Animator;
class KeysClipboardContainer;

class Key : public StdSelfRef {
public:
    Key(Animator *parentAnimator);
//    QrealPoint *mousePress(qreal frameT, qreal valueT,
//                    qreal pixelsPerFrame, qreal pixelsPerValue);
    virtual ~Key();

    virtual void startFrameTransform();
    virtual void finishFrameTransform();

    int getAbsFrame();
    virtual void setRelFrame(const int &frame);

    bool hasPrevKey();
    bool hasNextKey();
    void incFrameAndUpdateParentAnimator(const int &inc,
                                         const bool &finish = true);
    void setRelFrameAndUpdateParentAnimator(const int &relFrame,
                                         const bool &finish = true);

    Animator* getParentAnimator();

    virtual void mergeWith(const KeySPtr& key) { key->removeFromAnimator(); }

    virtual bool isDescendantSelected() { return isSelected(); }

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
    bool isSelected();

    virtual bool areAllChildrenSelected() {
        return false;
    }

    virtual void addToSelection(QList<KeyPtr> &selectedKeys,
                                QList<AnimatorQPtr> &selectedAnimators);
    virtual void removeFromSelection(QList<KeyPtr> &selectedKeys,
                                     QList<AnimatorQPtr> &selectedAnimators);

    bool isHovered() {
        return mHovered;
    }

    void setHovered(const bool &bT) {
        mHovered = bT;
    }

    int getRelFrame();
    void setAbsFrame(const int &frame);

    Key* getNextKey();
    Key *getPrevKey();

    bool differesFromNextKey() {
        return differsFromKey(getNextKey());
    }

    bool differesFromPrevKey() {
        return differsFromKey(getPrevKey());
    }

    virtual bool differsFromKey(Key* key) = 0;
    virtual void writeKey(QIODevice *target);
    virtual void readKey(QIODevice *target);
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
        key1->mergeWith(getAsSPtr(key2, Key));
    }

    Key* key1;
    Key* key2;
};

#endif // KEY_H
