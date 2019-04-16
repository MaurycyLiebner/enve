#ifndef POINTSHANDLER_H
#define POINTSHANDLER_H
#include "movablepoint.h"
#include "smartPointers/stdselfref.h"

class PointsHandler : public StdSelfRef {
    friend class StdSelfRef;
protected:
    PointsHandler();
public:
    virtual MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                                           const CanvasMode &canvasMode,
                                           const qreal &canvasScaleInv) {
        for(int i = mPts.count() - 1; i >= 0; i--) {
            const auto& pt = mPts.at(i);
            if(pt->isHidden()) continue;
            if(pt->isPointAtAbsPos(absPos, canvasScaleInv))
                return pt.get();
        }
        return nullptr;
    }

    void allPointsForSelection(QList<stdptr<MovablePoint>> &list,
                               const CanvasMode &currentCanvasMode) {
        for(const auto& pt : mPts) {
            if(pt->isHidden() || !pt->isSelected()) continue;
            if(pt->selectionEnabled()) list << pt.get();
        }
    }

    virtual void pointsInRectForSelection(const QRectF &absRect,
                                          const CanvasMode &currentCanvasMode,
                                          QList<stdptr<MovablePoint>> &list) const {
        for(const auto& pt : mPts) {
            if(!pt->selectionEnabled()) continue;
            if(pt->isSelected() && pt->isHidden()) continue;
            if(pt->isContainedInRect(absRect)) list.append(pt.get());
        }
    }

    virtual void drawPoints(SkCanvas * const canvas,
                            const CanvasMode &currentCanvasMode,
                            const SkScalar &invScale,
                            const bool& keyOnCurrentFrame) const {
        for(int i = mPts.count() - 1; i >= 0; i--) {
            const auto& pt = mPts.at(i);
            if(pt->isVisible()) pt->drawSk(canvas, invScale);
        }
    }

    template <class T>
    T* getPointWithId(const int& id) const {
        if(id < 0) return nullptr;
        if(id >= mPts.count()) return nullptr;
        return static_cast<T*>(mPts.at(id).get());
    }

    void removeAt(const int& id) {
        mPts.removeAt(id);
    }

    void clear() {
        mPts.clear();
    }

    void move(const int& from, const int& to) {
        mPts.move(from, to);
    }

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
private:
    QList<stdsptr<MovablePoint>> mPts;
};

#endif // POINTSHANDLER_H
