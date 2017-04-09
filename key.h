#ifndef KEY_H
#define KEY_H
#include "selfref.h"
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

    virtual KeyCloner *createNewKeyCloner() { return NULL; }

    virtual Key *makeKeyDuplicate(Animator *targetParent) {}

    virtual void startFrameTransform();
    virtual void finishFrameTransform();

    int getAbsFrame();
    virtual void setRelFrame(int frame);

    void changeFrameAndValueBy(QPointF frameValueChange);

    bool hasPrevKey();
    bool hasNextKey();
    void incFrameAndUpdateParentAnimator(int inc);

    Animator *getParentAnimator();

    virtual void mergeWith(Key *key) { key->removeFromAnimator(); }

    virtual bool isDescendantSelected() { return isSelected(); }

    void removeFromAnimator();

    virtual void deleteKey() { removeFromAnimator(); }

    void setParentKey(ComplexKey *parentKey);

    bool isAncestorSelected();

    int saveToSql(int parentAnimatorSqlId);
    void loadFromSql(int keyId);

    virtual void copyToContainer(KeysClipboardContainer *container);
    virtual void cancelFrameTransform();
    virtual void scaleFrameAndUpdateParentAnimator(const int &relativeToFrame,
                                                   const qreal &scaleFactor);
    virtual void setSelected(bool bT);
    bool isSelected();

    bool hasParentKey() {
        return mParentKey != NULL;
    }

    virtual bool areAllChildrenSelected() {
        return false;
    }

    virtual void addToSelection(QList<Key *> *selectedKeys);
    virtual void removeFromSelection(QList<Key *> *selectedKeys);

    bool isHovered() {
        return mHovered;
    }

    void setHovered(const bool &bT) {
        mHovered = bT;
    }

    int getRelFrame();
    void setAbsFrame(const int &frame);
    ComplexKey *getParentKey();

    Key *getNextKey();
    Key *getPrevKey();

    bool differesFromNextKey() {
        return differsFromKey(getNextKey());
    }

    bool differesFromPrevKey() {
        return differsFromKey(getPrevKey());
    }

    virtual bool differsFromKey(Key *key) = 0;
protected:
    bool mIsSelected = false;
    Animator *mParentAnimator = NULL;
    ComplexKey *mParentKey = NULL;

    int mRelFrame;
    int mSavedRelFrame;

    bool mHovered = false;
};

struct KeyPair {
    KeyPair(Key *key1T, Key *key2T) {
        key1 = key1T;
        key2 = key2T;
    }

    void merge() {
        key1->mergeWith(key2);
    }

    Key *key1;
    Key *key2;
};

class KeyCloner {
public:
    KeyCloner(Key *key);

    int getRelFrame() { return mRelFrame; }

    int getAbsFrame() { return mAbsFrame; }

    void shiftKeyFrame(const int &frameShift);

    virtual Key *createKeyForAnimator(Animator *parentAnimator) = 0;
protected:
    int mAbsFrame;
    int mRelFrame;
};

#endif // KEY_H
