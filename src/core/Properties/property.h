// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef PROPERTY_H
#define PROPERTY_H
class Canvas;
#include "../singlewidgettarget.h"
#include "../framerange.h"
#include "../MovablePoints/pointshandler.h"
#include "../ReadWrite/basicreadwrite.h"
#include "../conncontextptr.h"

class ComplexAnimator;
class Key;
class QPainter;
class UndoRedoStack;
class BasicTransformAnimator;
class BoxTransformAnimator;
enum class CanvasMode : short {
    boxTransform,
    pointTransform,
    pathCreate,
    paint,
    sculptPath,
    circleCreate,
    rectCreate,
    textCreate,
    pickFillStroke
};

enum class UpdateReason {
    frameChange,
    userChange
};

struct UndoRedo {
    std::function<void()> fUndo;
    std::function<void()> fRedo;
};

class Property;
template<typename T> class TypeMenu;
typedef TypeMenu<Property> PropertyMenu;

class Property : public SingleWidgetTarget {
    Q_OBJECT
    e_OBJECT
    e_DECLARE_TYPE(Property)
    friend class SceneParentSelfAssign;
protected:
    Property(const QString &name);

    virtual void prp_updateCanvasProps();
public:
    virtual int prp_getRelFrameShift() const { return 0; }

    virtual FrameRange prp_relInfluenceRange() const {
        return {FrameRange::EMIN, FrameRange::EMAX};
    }

    virtual FrameRange prp_getIdenticalRelRange(const int relFrame) const {
        Q_UNUSED(relFrame)
        return {FrameRange::EMIN, FrameRange::EMAX};
    }

    virtual void prp_drawTimelineControls(
            QPainter * const p, const qreal pixelsPerFrame,
            const FrameRange &absFrameRange, const int rowHeight) {
        Q_UNUSED(p)
        Q_UNUSED(pixelsPerFrame)
        Q_UNUSED(absFrameRange)
        Q_UNUSED(rowHeight)
    }

    virtual void prp_drawCanvasControls(
            SkCanvas * const canvas, const CanvasMode mode,
            const float invScale, const bool ctrlPressed);

    virtual void prp_setupTreeViewMenu(PropertyMenu * const menu);

    virtual void prp_cancelTransform() {}
    virtual void prp_startTransform() {}
    virtual void prp_finishTransform() {}

    virtual void prp_readProperty(eReadStream& src) { Q_UNUSED(src) }
    virtual void prp_writeProperty(eWriteStream& dst) const { Q_UNUSED(dst) }

    virtual int prp_getTotalFrameShift() const;
    virtual int prp_getInheritedFrameShift() const;

    virtual void prp_setInheritedFrameShift(const int shift,
                                            ComplexAnimator* parentAnimator);

    virtual void prp_afterFrameShiftChanged(const FrameRange& oldAbsRange,
                                            const FrameRange& newAbsRange);

    virtual BasicTransformAnimator *getTransformAnimator() const;

    virtual void prp_afterChangedAbsRange(const FrameRange &range,
                                          const bool clip = true);
public:
    QMatrix getTransform() const;

    void prp_setSelected(const bool selected);
    void prp_afterWholeInfluenceRangeChanged();
    void prp_afterChangedRelRange(const FrameRange &range,
                                  const bool clip = true);

    inline void prp_afterChangedCurrent(const UpdateReason reason) {
        emit prp_currentFrameChanged(reason, QPrivateSignal());
        if(reason == UpdateReason::frameChange && mParent_k)
            mParent_k->prp_afterChangedCurrent(reason);
    }

    FrameRange prp_getIdenticalAbsRange(const int relFrame) const;
    FrameRange prp_relRangeToAbsRange(const FrameRange &range) const;
    FrameRange prp_absRangeToRelRange(const FrameRange &range) const;
    int prp_absFrameToRelFrame(const int absFrame) const;
    int prp_relFrameToAbsFrame(const int relFrame) const;
    qreal prp_absFrameToRelFrameF(const qreal absFrame) const;
    qreal prp_relFrameToAbsFrameF(const qreal relFrame) const;
    const QString &prp_getName() const;
    void prp_setNameAction(const QString &newName);
    void prp_setName(const QString &newName);

    bool prp_differencesBetweenRelFrames(
            const int frame1, const int frame2) const;

    FrameRange prp_absInfluenceRange() const;

    //

    void prp_addUndoRedo(const UndoRedo &undoRedo);
    void prp_pushUndoRedoName(const QString& name);

    template <class T = ComplexAnimator>
    T *getParent() const {
        return static_cast<T*>(*mParent_k);
    }

    void setParent(ComplexAnimator * const parent);

    template <class T = Property>
    T *getFirstAncestor(const std::function<bool(Property*)>& tester) const {
        if(!mParent_k) return nullptr;
        if(tester(mParent_k)) return static_cast<T*>(*mParent_k);
        return mParent_k->getFirstAncestor<T>(tester);
    }

    template <class T = Property>
    T *getFirstAncestor() const {
        if(!mParent_k) return nullptr;
        const auto target = enve_cast<T*>(*mParent_k);
        if(target) return target;
        return mParent_k->getFirstAncestor<T>();
    }

    bool prp_isParentBoxSelected() const;

    bool prp_drawsOnCanvas() const
    { return mDrawOnCanvas; }

    PointsHandler * getPointsHandler() const
    { return mPointsHandler.get(); }

    Canvas* getParentScene() const
    { return mParentScene; }

    bool prp_isSelected() const { return prp_mSelected; }
    void prp_selectionChangeTriggered(const bool shiftPressed);

    void prp_enabledDrawingOnCanvas();
    void prp_disableDrawingOnCanvas();
    void prp_setDrawingOnCanvasEnabled(const bool enabled);

    void prp_getFullPath(QStringList& names) const;

    static QString prp_sFixName(const QString &name);
    static bool prp_sValidateName(const QString& name,
                                  QString* error = nullptr);
protected:
    void setPointsHandler(const stdsptr<PointsHandler>& handler);

    class SceneParentSelfAssign {
        friend class Canvas;
        SceneParentSelfAssign(Canvas * const scene) {
            reinterpret_cast<Property*>(scene)->mParentScene = scene;
        }
    };
signals:
    void prp_currentFrameChanged(const UpdateReason reason, QPrivateSignal);
    void prp_absFrameRangeChanged(const FrameRange &range, const bool clip);
    void prp_nameChanged(const QString&, QPrivateSignal);
    void prp_selectionChanged(bool, QPrivateSignal);
    void prp_parentChanged(ComplexAnimator*, QPrivateSignal);
    void prp_ancestorChanged(QPrivateSignal);
    void prp_pathChanged();
private:
    bool prp_mSelected = false;
    bool mDrawOnCanvas = false;
    int prp_mInheritedFrameShift = 0;
    QString prp_mName;
    ConnContextQPtr<Property> mParent_k;
    stdsptr<PointsHandler> mPointsHandler;
    Canvas* mParentScene = nullptr;
};

#endif // PROPERTY_H
