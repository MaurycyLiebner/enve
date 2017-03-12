#ifndef PROPERTY_H
#define PROPERTY_H
#include <QObject>
#include <QColor>
#include <QSqlQuery>
#include "connectedtomainwindow.h"
#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"

class ComplexAnimator;
class QrealKey;
class QrealAnimator;
class QPainter;
class AnimatorUpdater;

class Property :
    public QObject,
    public ConnectedToMainWindow,
    public SingleWidgetTarget {
    Q_OBJECT
public:
    Property();
    virtual ~Property() {}

    virtual void setUpdater(AnimatorUpdater *updater) {
        Q_UNUSED(updater);
    }

    virtual void callUpdater() {}

    virtual void drawKeys(QPainter *p, qreal pixelsPerFrame, qreal drawY,
                          int startFrame, int endFrame) {
        Q_UNUSED(p);
        Q_UNUSED(pixelsPerFrame);
        Q_UNUSED(drawY);
        Q_UNUSED(startFrame);
        Q_UNUSED(endFrame);
    }
    virtual void getKeysInRect(QRectF selectionRect,
                               qreal pixelsPerFrame,
                               QList<QrealKey *> *keysList) {
        Q_UNUSED(selectionRect);
        Q_UNUSED(pixelsPerFrame);
        Q_UNUSED(keysList);
    }

    virtual QrealKey *getKeyAtPos(qreal relX, int minViewedFrame,
                                  qreal pixelsPerFrame) {
        Q_UNUSED(relX);
        Q_UNUSED(minViewedFrame);
        Q_UNUSED(pixelsPerFrame);
    }

    virtual void cancelTransform() {}

    virtual void startTransform() {}

    virtual void finishTransform() {}

    virtual void retrieveSavedValue() {}

    virtual void sortKeys() {}

    virtual void setFrame(int frame) { Q_UNUSED(frame); }

    virtual void switchRecording() {}

    virtual void updateKeyOnCurrrentFrame() {}

    virtual bool isKeyOnCurrentFrame() { return false; }

    virtual bool isDescendantRecording() { return false; }

    virtual QString getValueText() {}

    virtual void clearFromGraphView() {}

    virtual void openContextMenu(QPoint pos) { Q_UNUSED(pos); }

    virtual int saveToSql(QSqlQuery*) {}

    virtual bool hasKeys() { return false; }

    ComplexAnimator *getParentAnimator() const { return mParentAnimator; }
    void setParentAnimator(ComplexAnimator *parentAnimator);
    void setZValue(const int &oldIndex, const int &newIndex);

    virtual bool isAnimator() { return false; }
    virtual void startDragging() {}

    virtual bool isRecording() { return false; }
    virtual void removeAllKeysFromComplexAnimator() {}
    virtual void setTransformed(bool bT) { Q_UNUSED(bT); }
    virtual void addAllKeysToComplexAnimator() {}

    QString getName();
    void setName(QString newName);
public slots:
    virtual void setRecording(bool rec) { Q_UNUSED(rec); }
protected:
    QString mName = "";
    ComplexAnimator *mParentAnimator = NULL;
};

#endif // PROPERTY_H
