#ifndef POINTSHANDLER_H
#define POINTSHANDLER_H
#include "movablepoint.h"
#include "smartPointers/stdselfref.h"

enum CanvasMode : short;

class PointsHandler : public StdSelfRef {
    friend class StdSelfRef;
protected:
    PointsHandler();
public:
    template <class T, typename... Args>
    T *createNewPt(const int& id, Args && ...args) {
        const auto newPt = SPtrCreateTemplated(T)(args...);
        mPts.insert(id, newPt);
        return newPt.get();
    }

    template <class T, typename... Args>
    T *createNewPt(Args && ...args) {
        return createNewPt<T>(mPts.count(), args...);
    }

    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                                   const qreal &canvasScaleInv,
                                   const CanvasMode &mode) {
        for(int i = mPts.count() - 1; i >= 0; i--) {
            const auto& pt = mPts.at(i);
            if(pt->isHidden(mode)) continue;
            if(pt->isPointAtAbsPos(absPos, canvasScaleInv))
                return pt.get();
        }
        return nullptr;
    }

    void addAllPointsToSelection(QList<stdptr<MovablePoint>> &selection,
                                 const CanvasMode &mode) {
        for(const auto& pt : mPts) {
            if(pt->isSelected() || pt->isHidden(mode)) continue;
            if(pt->selectionEnabled()) {
                pt->select();
                selection << pt.get();
            }
        }
    }

    void addInRectForSelection(const QRectF &absRect,
                               QList<stdptr<MovablePoint>> &selection,
                               const CanvasMode &mode) const {
        for(const auto& pt : mPts) {
            if(!pt->selectionEnabled()) continue;
            if(pt->isSelected() || pt->isHidden(mode)) continue;
            if(pt->isContainedInRect(absRect)) {
                pt->select();
                selection.append(pt.get());
            }
        }
    }

    void drawPoints(SkCanvas * const canvas,
                    const SkScalar &invScale,
                    const bool& keyOnCurrentFrame,
                    const CanvasMode &mode) const {
        for(int i = mPts.count() - 1; i >= 0; i--) {
            const auto& pt = mPts.at(i);
            if(pt->isVisible(mode)) pt->drawSk(canvas, invScale);
        }
    }

    template <class T>
    T* getPointWithId(const int& id) const {
        if(id < 0) return nullptr;
        if(id >= mPts.count()) return nullptr;
        return static_cast<T*>(mPts.at(id).get());
    }

    void removeAt(const int& id) { mPts.removeAt(id); }
    void clear() { mPts.clear(); }

    bool isEmpty() const { return mPts.isEmpty(); }
    int count() const { return mPts.count(); }

    void move(const int& from, const int& to) {
        mPts.move(from, to);
    }
private:
    QList<stdsptr<MovablePoint>> mPts;
};

#endif // POINTSHANDLER_H
