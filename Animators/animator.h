#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "Properties/property.h"

class ComplexAnimator;
class QrealKey;
class QrealAnimator;
class QPainter;
class AnimatorUpdater;

class Animator :
    public Property {
    Q_OBJECT
public:
    Animator();
    ~Animator() {}

    virtual void setUpdater(AnimatorUpdater *updater) = 0;

    virtual void callUpdater() = 0;

    virtual void drawKeys(QPainter *p, qreal pixelsPerFrame, qreal drawY,
                          int startFrame, int endFrame) = 0;
    virtual void getKeysInRect(QRectF selectionRect,
                               qreal pixelsPerFrame,
                               QList<QrealKey *> *keysList) = 0;

    virtual QrealKey *getKeyAtPos(qreal relX, int minViewedFrame,
                                  qreal pixelsPerFrame) = 0;

    virtual void cancelTransform() = 0;

    virtual void startTransform() = 0;

    virtual void finishTransform() = 0;

    virtual void retrieveSavedValue() = 0;

    virtual void sortKeys() = 0;

    virtual void setAbsFrame(int frame) = 0;

    virtual void switchRecording();

    virtual void updateKeyOnCurrrentFrame() = 0;

    virtual bool isKeyOnCurrentFrame() = 0;

    virtual bool isDescendantRecording() = 0;

    virtual QString getValueText() = 0;

    virtual void clearFromGraphView() = 0;

    virtual void openContextMenu(QPoint pos) = 0;

    virtual int saveToSql(QSqlQuery*) = 0;

    bool isCurrentAnimator() { return mIsCurrentAnimator; }

    bool isComplexAnimator() { return mIsComplexAnimator; }

    virtual bool hasKeys() = 0;

    virtual bool isAnimator() { return true; }
    virtual void startDragging() {}

    virtual int getFrameShift() const;
    void updateRelFrame();
    int absFrameToRelFrame(const int &absFrame) const;
    int relFrameToAbsFrame(const int &relFrame) const;
protected:
    int mCurrentAbsFrame = 0;
    int mCurrentRelFrame = 0;
    bool mIsComplexAnimator = false;
    bool mIsCurrentAnimator = false;
    bool mIsRecording = false;
    bool mKeyOnCurrentFrame = false;
    QColor mAnimatorColor;
public slots:
    virtual void setRecording(bool rec) = 0;
};

#endif // ANIMATOR_H
