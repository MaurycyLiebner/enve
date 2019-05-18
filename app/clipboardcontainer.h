#ifndef CLIPBOARDCONTAINER_H
#define CLIPBOARDCONTAINER_H
#include <QList>
#include <QWeakPointer>
#include "smartPointers/sharedpointerdefs.h"
class QrealAnimator;
class BoundingBox;
class LayerBox;
class KeysView;
class Key;
class Animator;
class Property;

typedef QPair<qptr<Animator>, QByteArray> AnimatorKeyDataPair;

enum  ClipboardContainerType : short {
    CCT_BOXES,
    CCT_KEYS,
    CCT_PROPERTY,
    CCT_COUNT
};

class ClipboardContainer : public StdSelfRef {
public:
    ClipboardContainer(const ClipboardContainerType &type);
    virtual ~ClipboardContainer();

    ClipboardContainerType getType();
    QByteArray *getBytesArray();
private:
    QByteArray mData;
    ClipboardContainerType mType;
};

class BoxesClipboardContainer : public ClipboardContainer {
    friend class StdSelfRef;
public:
    void pasteTo(LayerBox * const parent);
protected:
    BoxesClipboardContainer();
private:
    QList<qptr<BoundingBox>> mBoxesList;
};

class KeysClipboardContainer : public ClipboardContainer {
    friend class StdSelfRef;
public:
    ~KeysClipboardContainer();

    void paste(const int &pasteFrame,
               KeysView *keysView,
               const bool &merge,
               const bool &selectPasted);

    void addTargetAnimator(Animator *anim, const QByteArray& keyData);
protected:
    KeysClipboardContainer();
private:
    QList<AnimatorKeyDataPair> mAnimatorData;
};

class PropertyClipboardContainer : public ClipboardContainer {
    friend class StdSelfRef;
public:
    ~PropertyClipboardContainer();

    void paste(Property *targetProperty);
    void clearAndPaste(Property *targetProperty);

    bool propertyCompatible(Property *target);
    void setProperty(Property *property);

    bool isPathEffect() {
        return mPathEffect;
    }

    bool isPixmapEffect() {
        return mPixmapEffect;
    }

    bool isPathEffectAnimators() {
        return mPathEffectAnimators;
    }

    bool isPixmapEffectAnimators() {
        return mPixmapEffectAnimators;
    }
protected:
    PropertyClipboardContainer();
private:
    bool mQrealAnimator = false;
    bool mQPointFAnimator = false;
    bool mQStringAnimator = false;
    bool mPathAnimator = false;
    bool mAnimatedSurface = false;
    bool mComplexAnimator = false;
    bool mPathEffectAnimators = false;
    bool mPixmapEffectAnimators = false;
    bool mPathEffect = false;
    bool mPixmapEffect = false;
    bool mVectorPathAnimator = false;
    bool mBoxTargetProperty = false;
    QString mPropertyName;
    QPointer<BoundingBox> mTargetBox;
};

#endif // CLIPBOARDCONTAINER_H
