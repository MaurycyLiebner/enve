#ifndef PROPERTY_H
#define PROPERTY_H
#include <QObject>
#include <QColor>
class UndoRedo;
class MainWindow;
#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"

class ComplexAnimator;
class Key;
class QPainter;
class AnimatorUpdater;
class BoundingBox;

class InternalMimeData : public QMimeData {
public:
    enum Type : short {
        PIXMAP_EFFECT,
        PATH_EFFECT,
        BOUNDING_BOX
    };

    InternalMimeData(const Type& type) : mType(type) {}

    bool hasFormat(const QString &mimetype) const {
        if(mimetype == "av_internal_format") return true;
        return false;
    }

    bool hasType(const Type& type) const {
        return type == mType;
    }
private:
    const Type mType;
};

template <class T, InternalMimeData::Type type>
class PropertyMimeData : public InternalMimeData {
public:
    PropertyMimeData(T *target) :
        InternalMimeData(type), mTarget(target) {}

    T *getTarget() const {
        return mTarget;
    }
private:
    const QPointer<T> mTarget;
};

class Property : public SingleWidgetTarget {
    Q_OBJECT
public:
    virtual ~Property() {
        emit beingDeleted();
    }

    virtual void prp_valueChanged();

    virtual int prp_getRelFrameShift() const {
        return 0;
    }
    virtual int prp_getFrameShift() const;
    virtual int prp_getParentFrameShift() const;

    int prp_absFrameToRelFrame(const int &absFrame) const;
    int prp_relFrameToAbsFrame(const int &relFrame) const;
    qreal prp_absFrameToRelFrameF(const qreal &absFrame) const;
    qreal prp_relFrameToAbsFrameF(const qreal &relFrame) const;

    virtual void prp_drawKeys(QPainter *p,
                              const qreal &pixelsPerFrame,
                              const qreal &drawY,
                              const int &startFrame,
                              const int &endFrame) {
        Q_UNUSED(p);
        Q_UNUSED(pixelsPerFrame);
        Q_UNUSED(drawY);
        Q_UNUSED(startFrame);
        Q_UNUSED(endFrame);
    }
    virtual void prp_getKeysInRect(const QRectF &selectionRect,
                                   const qreal &pixelsPerFrame,
                                   QList<Key*>& keysList) {
        Q_UNUSED(selectionRect);
        Q_UNUSED(pixelsPerFrame);
        Q_UNUSED(keysList);
    }

    virtual Key *prp_getKeyAtPos(const qreal &relX,
                                 const int &minViewedFrame,
                                 const qreal &pixelsPerFrame) {
        Q_UNUSED(relX);
        Q_UNUSED(minViewedFrame);
        Q_UNUSED(pixelsPerFrame);
        return nullptr;
    }

    virtual void prp_cancelTransform() {}

    virtual void prp_startTransform() {}

    virtual void prp_finishTransform() {}

    virtual void prp_retrieveSavedValue() {}

    virtual void prp_setAbsFrame(const int &frame) { Q_UNUSED(frame); }

    virtual void prp_switchRecording() {}

    virtual void anim_updateKeyOnCurrrentFrame() {}

    virtual bool prp_isKeyOnCurrentFrame() { return false; }

    virtual bool prp_isDescendantRecording() { return false; }

    virtual QString prp_getValueText() { return ""; }

    virtual void prp_clearFromGraphView() {}

    virtual void prp_openContextMenu(const QPoint &pos) { Q_UNUSED(pos); }

    virtual bool prp_hasKeys() { return false; }

    virtual bool prp_isAnimator() { return false; }
    virtual void prp_startDragging() {}

    virtual bool prp_isRecording() { return false; }
    virtual void prp_removeAllKeysFromComplexAnimator(
            ComplexAnimator* target) {
        Q_UNUSED(target);
    }
    virtual void prp_setTransformed(const bool &bT) { Q_UNUSED(bT); }
    virtual void prp_addAllKeysToComplexAnimator(
            ComplexAnimator* target) {
        Q_UNUSED(target);
    }

    const QString &prp_getName();
    void prp_setName(const QString &newName);

    virtual void prp_setUpdater(const AnimatorUpdaterSPtr &updater);
    void prp_blockUpdater();
    void prp_callFinishUpdater();

    virtual void prp_setParentFrameShift(const int &shift,
                                         ComplexAnimator* parentAnimator = nullptr);
    void prp_setBlockedUpdater(const AnimatorUpdaterSPtr &updater);

    bool SWT_isProperty() { return true; }

    virtual bool prp_differencesBetweenRelFrames(const int &,
                                                 const int &) {
        return false;
    }

    virtual void prp_getFirstAndLastIdenticalRelFrame(
            int *firstIdentical,
            int *lastIdentical,
            const int &relFrame) {
        *firstIdentical = INT_MIN;
        *lastIdentical = INT_MAX;
        Q_UNUSED(relFrame);
    }

    virtual bool prp_nextRelFrameWithKey(const int &relFrame,
                                         int &nextRelFrame) {
        Q_UNUSED(relFrame);
        Q_UNUSED(nextRelFrame);
        return false;
    }

    virtual bool prp_prevRelFrameWithKey(const int &relFrame,
                                         int &prevRelFrame) {
        Q_UNUSED(relFrame);
        Q_UNUSED(prevRelFrame);
        return false;
    }

    virtual void readProperty(QIODevice *device) {
        Q_UNUSED(device);
    }

    virtual void writeProperty(QIODevice *device) {
        Q_UNUSED(device);
    }

    AnimatorUpdater *prp_getUpdater() {
        return prp_mUpdater.get();
    }

    //

    void addUndoRedo(UndoRedo *undoRedo);
    void callUpdateSchedulers();
    MainWindow *getMainWindow();
    virtual void schedulePivotUpdate();
    bool isShiftPressed();
    bool isCtrlPressed();
    bool isAltPressed();

    int getCurrentFrameFromMainWindow();
    int getFrameCount();
    void graphUpdateAfterKeysChanged();
    void graphScheduleUpdateAfterKeysChanged();
    bool isShiftPressed(QKeyEvent *event);
    bool isCtrlPressed(QKeyEvent *event);
    bool isAltPressed(QKeyEvent *event);
    Property *getLastSetParent() {
        if(mLastSetParent == nullptr) return nullptr;
        return mLastSetParent;
    }

    void setLastSetParent(Property *parent) {
        mLastSetParent = parent;
    }

    BoundingBox *getLastSetParentBoundingBoxAncestor();
protected:
    Property(const QString &name);

    MainWindow *mMainWindow;
    QPointer<Property> mLastSetParent;
public slots:
    void prp_callUpdater();

    virtual void prp_setRecording(const bool &rec) { Q_UNUSED(rec); }

    virtual void prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                                     const int &maxFrame);

    virtual void prp_updateAfterChangedRelFrameRange(const int &minFrame,
                                                     const int &maxFrame) {
        int minFrameT = minFrame;
        int maxFrameT = maxFrame;

        if(minFrameT != INT_MIN && minFrameT != INT_MAX) {
            minFrameT = prp_relFrameToAbsFrame(minFrameT);
        }
        if(maxFrameT != INT_MAX && maxFrameT != INT_MIN) {
            maxFrameT = prp_relFrameToAbsFrame(maxFrameT);
        }

        if(maxFrameT < minFrameT) {
            prp_updateAfterChangedAbsFrameRange(maxFrameT,
                                                minFrameT);
        } else {
            prp_updateAfterChangedAbsFrameRange(minFrameT,
                                                maxFrameT);
        }
    }

    virtual void prp_updateInfluenceRangeAfterChanged();
signals:
    void prp_updateWholeInfluenceRange();
    void prp_isRecordingChanged();
    void prp_absFrameRangeChanged(const int &minFrame,
                                  const int &maxFrame);
    void prp_removingKey(Key*);
    void prp_addingKey(Key*);
    void prp_replaceWith(const PropertyQSPtr&, const PropertyQSPtr&);
    void prp_prependWith(Property*, const PropertyQSPtr&);
    void beingDeleted();
protected:
    bool prp_mUpdaterBlocked = false;
    int prp_mParentFrameShift = 0;
    AnimatorUpdaterSPtr prp_mUpdater;
    QString prp_mName = "";
};

#endif // PROPERTY_H
