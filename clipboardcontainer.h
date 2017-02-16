#ifndef CLIPBOARDCONTAINER_H
#define CLIPBOARDCONTAINER_H
#include <QList>
class QrealKey;
class QrealAnimator;
class BoundingBox;
class BoxesGroup;

enum  ClipboardContainerType : short {
    CCT_BOXES,
    CCT_KEYS,
    CCT_COUNT
};

class ClipboardContainer {
public:
    ClipboardContainer(const ClipboardContainerType &type);
    virtual ClipboardContainer();

    ClipboardContainerType getType();
private:
    ClipboardContainerType mType;
};

class BoxesClipboardContainer {
public:
    BoxesClipboardContainer();
    ~BoxesClipboardContainer();

    void copyBoxToContainer(BoundingBox *box);

    void pasteTo(BoxesGroup *parent);
private:
    QList<BoundingBox*> mBoxesList;
};

class KeysClipboardContainer {
public:
    KeysClipboardContainer();
    ~KeysClipboardContainer();

    void copyKeyToContainer(QrealKey *key);

    void paste(const int &pasteFrame);
private:
    QList<BoundingBox*> mKeysList;
    QList<QrealAnimator*> mTargetAnimators;
};

#endif // CLIPBOARDCONTAINER_H
