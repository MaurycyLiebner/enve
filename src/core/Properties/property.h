#ifndef PROPERTY_H
#define PROPERTY_H
class UndoRedo;
class MainWindow;
#include "../singlewidgettarget.h"
#include "../framerange.h"
#include "../MovablePoints/pointshandler.h"
#include "../basicreadwrite.h"

class ComplexAnimator;
class Key;
class QPainter;
class PropertyUpdater;
class UndoRedoStack;
class BasicTransformAnimator;
class BoxTransformAnimator;
enum class CanvasMode : short {
    boxTransform,
    pointTransform,
    pathCreate,
    paint,
    pickFillStroke,
    circleCreate,
    rectCreate,
    textCreate
};

class eDraggedObjects {
public:
    template <typename T>
    eDraggedObjects(const QList<T*>& objs) :
        mMetaObj(T::staticMetaObject) {
        for(const auto& obj : objs) mObjects << obj;
    }

    template <typename T>
    bool hasType() const {
        return &mMetaObj == &T::staticMetaObject;
    }

    template <typename T>
    QList<T*> getObjects() const {
        if(!hasType<T>()) RuntimeThrow("Incompatible type");
        QList<T*> result;
        for(const auto& obj : mObjects) {
            const auto ObjT = qobject_cast<T*>(obj);
            if(ObjT) result << ObjT;
        }
        return result;
    }

    template <typename T>
    T* getObject(const int index) const {
        return qobject_cast<T*>(mObjects.at(index));
    }

    int count() const { return mObjects.count(); }

    bool hasObject(QObject* const obj) const {
        for(const auto& iObj : mObjects) {
            if(iObj == obj) return true;
        }
        return false;
    }
private:
    const QMetaObject& mMetaObj;
    QList<QObject*> mObjects;
};

class eMimeData : public QMimeData, public eDraggedObjects {
public:
    template <typename T>
    eMimeData(const QList<T*>& objs) : eDraggedObjects(objs) {}

    template <typename T>
    static bool sHasType(const QMimeData* const data) {
        if(!data->hasFormat("eMimeData")) return false;
        return static_cast<const eMimeData*>(data)->hasType<T>();
    }
protected:
    bool hasFormat(const QString &mimetype) const {
        if(mimetype == "eMimeData") return true;
        return false;
    }
};

class Property;
template<typename T> class TypeMenu;
typedef TypeMenu<Property> PropertyMenu;

class Property : public SingleWidgetTarget {
    Q_OBJECT
protected:
    Property(const QString &name);

    virtual void updateCanvasProps() {
        if(mParent_k) mParent_k->updateCanvasProps();
    }
public:
    virtual int prp_getRelFrameShift() const {
        return 0;
    }

    virtual void drawTimelineControls(QPainter * const p,
                                      const qreal pixelsPerFrame,
                                      const FrameRange &absFrameRange,
                                      const int rowHeight) {
        Q_UNUSED(p);
        Q_UNUSED(pixelsPerFrame);
        Q_UNUSED(absFrameRange);
        Q_UNUSED(rowHeight);
    }

    virtual void drawCanvasControls(SkCanvas * const canvas,
                                    const CanvasMode mode,
                                    const float invScale,
                                    const bool ctrlPressed);

    virtual void setupTreeViewMenu(PropertyMenu * const menu);

    virtual int prp_getTotalFrameShift() const;
    virtual int prp_getInheritedFrameShift() const;

    virtual void prp_cancelTransform() {}

    virtual void prp_startTransform() {}

    virtual void prp_finishTransform() {}

    virtual QString prp_getValueText() { return ""; }

    virtual void prp_setTransformed(const bool bT) { Q_UNUSED(bT); }

    virtual void prp_setInheritedFrameShift(const int shift,
                                            ComplexAnimator* parentAnimator);
    virtual void prp_afterFrameShiftChanged(const FrameRange& oldAbsRange,
                                            const FrameRange& newAbsRange) {
        prp_afterChangedAbsRange(newAbsRange + oldAbsRange, false);
    }

    virtual FrameRange prp_getIdenticalRelRange(const int relFrame) const {
        Q_UNUSED(relFrame);
        return {FrameRange::EMIN, FrameRange::EMAX};
    }

    virtual void readProperty(QIODevice * const src) {
        Q_UNUSED(src);
    }

    virtual void writeProperty(QIODevice * const dst) const {
        Q_UNUSED(dst);
    }

    virtual BasicTransformAnimator *getTransformAnimator() const {
        if(mParent_k) return mParent_k->getTransformAnimator();
        return nullptr;
    }

    virtual void prp_afterChangedAbsRange(const FrameRange &range,
                                          const bool clip = true);
protected:
    virtual void prp_setUpdater(const stdsptr<PropertyUpdater>& updater);
public:
    bool SWT_isProperty() const { return true; }

    QMatrix getTransform() const;

    void prp_afterWholeInfluenceRangeChanged();

    void prp_afterChangedRelRange(const FrameRange &range,
                                  const bool clip = true) {
        const auto absRange = prp_relRangeToAbsRange(range);
        prp_afterChangedAbsRange(absRange, clip);
    }

    FrameRange prp_relRangeToAbsRange(const FrameRange &range) const;
    FrameRange prp_absRangeToRelRange(const FrameRange &range) const;
    int prp_absFrameToRelFrame(const int absFrame) const;
    int prp_relFrameToAbsFrame(const int relFrame) const;
    qreal prp_absFrameToRelFrameF(const qreal absFrame) const;
    qreal prp_relFrameToAbsFrameF(const qreal relFrame) const;
    const QString &prp_getName() const;
    void prp_setName(const QString &newName);

    void prp_setOwnUpdater(const stdsptr<PropertyUpdater> &updater);
    void prp_setInheritedUpdater(const stdsptr<PropertyUpdater> &updater);

    bool prp_differencesBetweenRelFrames(const int frame1,
                                         const int frame2) const {
        return !prp_getIdenticalRelRange(frame1).inRange(frame2);
    }

    FrameRange prp_absInfluenceRange() const {
        return prp_relRangeToAbsRange(prp_relInfluenceRange());
    }

    virtual FrameRange prp_relInfluenceRange() const {
        return {FrameRange::EMIN, FrameRange::EMAX};
    }

    PropertyUpdater *prp_getUpdater() const {
        return prp_mUpdater.get();
    }

    //

    void addUndoRedo(const stdsptr<UndoRedo> &undoRedo);

    template <class T = ComplexAnimator>
    T *getParent() const {
        return static_cast<T*>(mParent_k.data());
    }

    void setParent(ComplexAnimator * const parent);

    template <class T = Property>
    T *getFirstAncestor(const std::function<bool(Property*)>& tester) const {
        if(!mParent_k) return nullptr;
        if(tester(mParent_k)) return static_cast<T*>(mParent_k.data());
        return mParent_k->getFirstAncestor<T>(tester);
    }

    template <class T = Property>
    T *getFirstAncestor() const {
        if(!mParent_k) return nullptr;
        const auto target = dynamic_cast<T*>(mParent_k.data());
        if(target) return target;
        return mParent_k->getFirstAncestor<T>();
    }

    bool drawsOnCanvas() const {
        return mDrawOnCanvas;
    }

    PointsHandler * getPointsHandler() const {
        return mPointsHandler.get();
    }
protected:
    void prp_callUpdater();
    void prp_callFinishUpdater();
    void enabledDrawingOnCanvas();
    void setPointsHandler(const stdsptr<PointsHandler>& handler);
signals:
    void prp_absFrameRangeChanged(const FrameRange &range,
                                  const bool clip);
    void prp_replaceWith(const qsptr<Property>&, const qsptr<Property>&);
    void prp_prependWith(Property*, const qsptr<Property>&);
    void prp_nameChanged(const QString&);
protected:
    bool prp_mOwnUpdater = false;
    bool mDrawOnCanvas = false;
    int prp_mInheritedFrameShift = 0;
    stdsptr<PropertyUpdater> prp_mUpdater;
    QString prp_mName;
    stdptr<UndoRedoStack> mParentCanvasUndoRedoStack;
    qptr<Property> mParent_k;
    stdsptr<PointsHandler> mPointsHandler;
};

#endif // PROPERTY_H
