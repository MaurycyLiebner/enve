#ifndef CLIPBOARDCONTAINER_H
#define CLIPBOARDCONTAINER_H
#include <QList>
class QrealKey;
class QrealAnimator;
class BoundingBox;
class BoxesGroup;
class KeysView;

enum  ClipboardContainerType : short {
    CCT_BOXES,
    CCT_KEYS,
    CCT_ANIMATOR,
    CCT_COUNT
};

class ClipboardContainer {
public:
    ClipboardContainer(const ClipboardContainerType &type);
    virtual ~ClipboardContainer() {}

    ClipboardContainerType getType();
private:
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

    void copyKeyToContainer(QrealKey *key);

    void paste(const int &pasteFrame,
               KeysView *keysView);
private:
    QList<QrealKey*> mKeysList;
    QList<QrealAnimator*> mTargetAnimators;
};

class AnimatorClipboardContainer : public ClipboardContainer {
public:
    AnimatorClipboardContainer();
    ~AnimatorClipboardContainer();

    void setAnimator(QrealAnimator *animator);

    void paste(QrealAnimator *target);
private:
    QrealAnimator *mAnimator;
};

#endif // CLIPBOARDCONTAINER_H
