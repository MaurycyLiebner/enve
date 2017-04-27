#ifndef PROPERTY_H
#define PROPERTY_H
#include <QObject>
#include <QColor>
#include <QSqlQuery>
#include "connectedtomainwindow.h"
#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"
#include "selfref.h"

class ComplexAnimator;
class Key;
class QrealAnimator;
class QPainter;
class AnimatorUpdater;

class Property :
    public QObject,
    public ConnectedToMainWindow,
    public SingleWidgetTarget,
    public SelfRef {
    Q_OBJECT
public:
    Property();
    virtual ~Property() {}

    virtual void prp_valueChanged();

    virtual int prp_getFrameShift() const;
    virtual int prp_getParentFrameShift() const;

    virtual void prp_updateAfterChangedRelFrameRange(const int &minFrame,
                                                     const int &maxFrame) {
        int minFrameT;
        if(minFrame == INT_MIN) {
            minFrameT = minFrame;
        } else {
            minFrameT = prp_relFrameToAbsFrame(minFrame);
        }
        int maxFrameT;
        if(maxFrame == INT_MAX) {
            maxFrameT = maxFrame;
        } else {
            maxFrameT = prp_relFrameToAbsFrame(maxFrame);
        }
        prp_updateAfterChangedAbsFrameRange(minFrameT,
                                            maxFrameT);
    }

    int prp_absFrameToRelFrame(const int &absFrame) const;
    int prp_relFrameToAbsFrame(const int &relFrame) const;

    int prp_absFrameToParentRelFrame(const int &absFrame) const;
    int prp_parentRelFrameToAbsFrame(const int &relFrame) const;

    virtual void prp_drawKeys(QPainter *p,
                              qreal pixelsPerFrame, qreal drawY,
                              int startFrame, int endFrame) {
        Q_UNUSED(p);
        Q_UNUSED(pixelsPerFrame);
        Q_UNUSED(drawY);
        Q_UNUSED(startFrame);
        Q_UNUSED(endFrame);
    }
    virtual void prp_getKeysInRect(QRectF selectionRect,
                                   qreal pixelsPerFrame,
                                   QList<Key *> *keysList) {
        Q_UNUSED(selectionRect);
        Q_UNUSED(pixelsPerFrame);
        Q_UNUSED(keysList);
    }

    virtual Key *prp_getKeyAtPos(qreal relX, int minViewedFrame,
                                 qreal pixelsPerFrame) {
        Q_UNUSED(relX);
        Q_UNUSED(minViewedFrame);
        Q_UNUSED(pixelsPerFrame);
        return NULL;
    }

    virtual void prp_cancelTransform() {}

    virtual void prp_startTransform() {}

    virtual void prp_finishTransform() {}

    virtual void prp_retrieveSavedValue() {}

    virtual void prp_setAbsFrame(int frame) { Q_UNUSED(frame); }

    virtual void prp_switchRecording() {}

    virtual void anim_updateKeyOnCurrrentFrame() {}

    virtual bool prp_isKeyOnCurrentFrame() { return false; }

    virtual bool prp_isDescendantRecording() { return false; }

    virtual QString prp_getValueText() { return ""; }

    virtual void prp_clearFromGraphView() {}

    virtual void prp_openContextMenu(QPoint pos) { Q_UNUSED(pos); }

    virtual bool prp_hasKeys() { return false; }

    virtual bool prp_isAnimator() { return false; }
    virtual void prp_startDragging() {}

    virtual bool prp_isRecording() { return false; }
    virtual void prp_removeAllKeysFromComplexAnimator(ComplexAnimator *target) {
        Q_UNUSED(target);
    }
    virtual void prp_setTransformed(bool bT) { Q_UNUSED(bT); }
    virtual void prp_addAllKeysToComplexAnimator(ComplexAnimator *target) {
        Q_UNUSED(target);
    }

    QString prp_getName();
    void prp_setName(QString newName);

    virtual void prp_setUpdater(AnimatorUpdater *updater);
    virtual void prp_callUpdater();
    void prp_blockUpdater();
    void prp_callFinishUpdater();

    virtual void prp_setParentFrameShift(const int &shift);
    void prp_setBlockedUpdater(AnimatorUpdater *updater);
public slots:
    virtual void prp_setRecording(bool rec) { Q_UNUSED(rec); }

    virtual void prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                     const int &maxFrame);
signals:
    void prp_isRecordingChanged();
    void prp_absFrameRangeChanged(const int &minFrame,
                                  const int &maxFrame);
    void prp_removingKey(Key *);
    void prp_addingKey(Key *);
protected:
    int prp_mParentFrameShift = 0;
    AnimatorUpdater *prp_mUpdater = NULL;
    bool prp_mUpdaterBlocked = false;

    QString prp_mName = "";
};

#endif // PROPERTY_H
