#ifndef PROPERTY_H
#define PROPERTY_H
class UndoRedo;
class MainWindow;
#include "singlewidgettarget.h"
#include "glhelpers.h"
#include "framerange.h"
#include "MovablePoints/pointshandler.h"
#include "basicreadwrite.h"

class ComplexAnimator;
class Key;
class QPainter;
class PropertyUpdater;
class UndoRedoStack;
class BasicTransformAnimator;
class BoxTransformAnimator;
enum CanvasMode : short {
    MOVE_BOX,
    MOVE_POINT,
    ADD_POINT,
    PICK_PAINT_SETTINGS,
    ADD_CIRCLE,
    ADD_RECTANGLE,
    ADD_TEXT,
    ADD_PARTICLE_BOX,
    ADD_PARTICLE_EMITTER,
    ADD_PAINT_BOX,
    PAINT_MODE
};

//! @brief Use only as base class for PropertyMimeData.
class InternalMimeData : public QMimeData {
public:
    enum Type : short {
        PIXMAP_EFFECT,
        PATH_EFFECT,
        BOUNDING_BOX
    };

    InternalMimeData(const Type& type) : mType(type) {}

    bool hasType(const Type& type) const {
        return type == mType;
    }
protected:
    bool hasFormat(const QString &mimetype) const {
        if(mimetype == "av_internal_format") return true;
        return false;
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

    static bool hasFormat(const QMimeData * const data) {
        if(!data->hasFormat("av_internal_format")) return false;
        auto internalData = static_cast<const InternalMimeData*>(data);
        return internalData->hasType(type);
    }
private:
    const QPointer<T> mTarget;
};

class Property;
template<typename T> class TypeMenu;
typedef TypeMenu<Property> PropertyTypeMenu;

class Property : public SingleWidgetTarget {
    Q_OBJECT
protected:
    Property(const QString &name);
public:
    ~Property() { emit beingDeleted(); }

    bool SWT_isProperty() const { return true; }

    virtual int prp_getRelFrameShift() const {
        return 0;
    }

    virtual void drawTimelineControls(QPainter * const p,
                                      const qreal &pixelsPerFrame,
                                      const FrameRange &absFrameRange,
                                      const int &rowHeight) {
        Q_UNUSED(p);
        Q_UNUSED(pixelsPerFrame);
        Q_UNUSED(absFrameRange);
        Q_UNUSED(rowHeight);
    }

    virtual void drawCanvasControls(SkCanvas * const canvas,
                                    const CanvasMode &mode,
                                    const SkScalar &invScale);

    virtual void addActionsToMenu(PropertyTypeMenu * const menu) {
        Q_UNUSED(menu);
    }

    virtual int prp_getFrameShift() const;
    virtual int prp_getParentFrameShift() const;

    virtual void prp_cancelTransform() {}

    virtual void prp_startTransform() {}

    virtual void prp_finishTransform() {}

    virtual QString prp_getValueText() { return ""; }

    virtual void prp_startDragging() {}

    virtual void prp_setTransformed(const bool &bT) { Q_UNUSED(bT); }

    virtual void prp_setParentFrameShift(const int &shift,
                                         ComplexAnimator* parentAnimator = nullptr);
    virtual void prp_afterFrameShiftChanged() {}
    virtual FrameRange prp_getIdenticalRelRange(const int &relFrame) const {
        Q_UNUSED(relFrame);
        return {FrameRange::EMIN, FrameRange::EMAX};
    }

    virtual void readProperty(QIODevice *device) {
        Q_UNUSED(device);
    }

    virtual void writeProperty(QIODevice * const device) const {
        Q_UNUSED(device);
    }

    QMatrix getTransform() const;

    virtual BasicTransformAnimator *getTransformAnimator() const {
        if(mParent) return mParent->getTransformAnimator();
        return nullptr;
    }

    virtual void prp_afterChangedAbsRange(const FrameRange &range);
protected:
    virtual void prp_setUpdater(const stdsptr<PropertyUpdater>& updater);
public:
    void prp_afterWholeInfluenceRangeChanged();

    void prp_afterChangedRelRange(const FrameRange &range) {
        const auto absRange = prp_relRangeToAbsRange(range);
        prp_afterChangedAbsRange(absRange);
    }

    FrameRange prp_relRangeToAbsRange(const FrameRange &range) const;
    FrameRange prp_absRangeToRelRange(const FrameRange &range) const;
    int prp_absFrameToRelFrame(const int &absFrame) const;
    int prp_relFrameToAbsFrame(const int &relFrame) const;
    qreal prp_absFrameToRelFrameF(const qreal &absFrame) const;
    qreal prp_relFrameToAbsFrameF(const qreal &relFrame) const;
    const QString &prp_getName() const;
    void prp_setName(const QString &newName);

    void prp_setOwnUpdater(const stdsptr<PropertyUpdater> &updater);
    void prp_setInheritedUpdater(const stdsptr<PropertyUpdater> &updater);

    bool prp_differencesBetweenRelFrames(const int &frame1,
                                         const int &frame2) const {
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
        return static_cast<T*>(mParent.data());
    }

    void setParent(ComplexAnimator * const parent);

    template <class T = Property>
    T *getFirstAncestor(const std::function<bool(Property*)>& tester) const {
        if(!mParent) return nullptr;
        if(tester(mParent.data())) return static_cast<T*>(mParent.data());
        return mParent->getFirstAncestor<T>(tester);
    }

    template <class T = Property>
    T *getFirstAncestor() const {
        if(!mParent) return nullptr;
        const auto target = dynamic_cast<T*>(mParent.data());
        if(target) return target;
        return mParent->getFirstAncestor<T>();
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

    virtual void updateCanvasProps() {
        if(mParent) mParent->updateCanvasProps();
    }
    void enabledDrawingOnCanvas();
    void setPointsHandler(const stdsptr<PointsHandler>& handler);
signals:
    void prp_absFrameRangeChanged(const FrameRange &range);
    void prp_removingKey(Key*);
    void prp_addingKey(Key*);
    void prp_replaceWith(const qsptr<Property>&, const qsptr<Property>&);
    void prp_prependWith(Property*, const qsptr<Property>&);
    void beingDeleted();
protected:
    bool prp_mOwnUpdater = false;
    bool mDrawOnCanvas = false;
    int prp_mParentFrameShift = 0;
    stdsptr<PropertyUpdater> prp_mUpdater;
    QString prp_mName = "";
    stdptr<UndoRedoStack> mParentCanvasUndoRedoStack;
    QPointer<Property> mParent;
    stdsptr<PointsHandler> mPointsHandler;
};

#endif // PROPERTY_H
