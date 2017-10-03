#ifndef CLIPBOARDCONTAINER_H
#define CLIPBOARDCONTAINER_H
#include <QList>
class QrealAnimator;
class BoundingBox;
class BoxesGroup;
class KeysView;
class Key;
class KeyCloner;
class Animator;
class Property;

enum  ClipboardContainerType : short {
    CCT_BOXES,
    CCT_KEYS,
    CCT_PROPERTY,
    CCT_COUNT
};

class ClipboardContainer {
public:
    ClipboardContainer(const ClipboardContainerType &type);
    virtual ~ClipboardContainer() {}

    ClipboardContainerType getType();
    QByteArray *getBytesArray() {
        return &mData;
    }
private:
    QByteArray mData;
    ClipboardContainerType mType;
};

class BoxesClipboardContainer : public ClipboardContainer {
public:
    BoxesClipboardContainer();
    ~BoxesClipboardContainer();

    void copyBoxToContainer(BoundingBox *box);

    void pasteTo(BoxesGroup *parent);
private:
    QList<BoundingBox*> mBoxesList;
};

class KeysClipboardContainer : public ClipboardContainer {
public:
    KeysClipboardContainer();
    ~KeysClipboardContainer();

    void paste(const int &pasteFrame,
               KeysView *keysView);

    void addTargetAnimator(Animator *anim);
private:
    QList<QWeakPointer<Animator> > mTargetAnimators;
};

class PropertyClipboardContainer : public ClipboardContainer {
public:
    PropertyClipboardContainer();
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
    QString mPropertyName;
};

#endif // CLIPBOARDCONTAINER_H
