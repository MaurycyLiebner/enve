#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <QObject>
#include <QColor>
#include "connectedtomainwindow.h"

class ComplexAnimator;
class QrealKey;
class QrealAnimator;
class QPainter;
class AnimatorUpdater;

class Animator :  public QObject, public ConnectedToMainWindow
{
    Q_OBJECT
public:
    Animator();
    ~Animator() {}

    virtual void setUpdater(AnimatorUpdater *updater) = 0;

    virtual void callUpdater() = 0;

    virtual void drawKeys(QPainter *p, qreal pixelsPerFrame, qreal startY,
                          int startFrame, int endFrame) = 0;
    virtual void getKeysInRect(QRectF selectionRect,
                               int minViewedFrame,
                               qreal pixelsPerFrame,
                               QList<QrealKey *> *keysList) = 0;

    virtual QrealKey *getKeyAtPos(qreal relX, int minViewedFrame,
                                  qreal pixelsPerFrame) = 0;

    virtual void cancelTransform() = 0;

    virtual void startTransform() = 0;

    virtual void finishTransform() = 0;

    virtual void retrieveSavedValue() = 0;

    virtual void sortKeys() = 0;

    virtual void setFrame(int frame) = 0;

    virtual void setRecording(bool rec) = 0;

    virtual void updateKeyOnCurrrentFrame() = 0;

    virtual bool isKeyOnCurrentFrame() = 0;

    virtual bool isDescendantRecording() = 0;

    virtual QString getValueText() = 0;

    virtual void clearFromGraphView() = 0;

    virtual void openContextMenu(QPoint pos) = 0;

    virtual int saveToSql() = 0;

    bool isCurrentAnimator() { return mIsCurrentAnimator; }

    bool isComplexAnimator() { return mIsComplexAnimator; }

    virtual bool hasKeys() = 0;

    ComplexAnimator *getParentAnimator() const { return mParentAnimator; }
    void setParentAnimator(ComplexAnimator *parentAnimator);
    void setZValue(const int &oldIndex, const int &newIndex);

    virtual bool isEffectsAnimator() { return false; }
    virtual void startDragging() {}
protected:
    bool mIsComplexAnimator = false;
    bool mIsCurrentAnimator = false;
    bool mIsRecording = false;
    bool mKeyOnCurrentFrame = false;
    QString mName = "";
    ComplexAnimator *mParentAnimator = NULL;
    QColor mAnimatorColor;
signals:
    void childAnimatorAdded(QrealAnimator*);
    void childAnimatorRemoved(QrealAnimator*);
    void childAnimatorZChanged(int, int);
public slots:
};

#endif // ANIMATOR_H
