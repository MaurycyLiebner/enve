#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "skia/skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"

class BoundingBox;
class NodePoint;
class BasicTransformAnimator;
enum CanvasMode : short;
class PropertyUpdater;

enum MovablePointType : short {
    TYPE_PATH_POINT,
    TYPE_SMART_PATH_POINT,
    TYPE_CTRL_POINT,
    TYPE_PIVOT_POINT,
    TYPE_GRADIENT_POINT
};
class MovablePoint;
template<typename T> class TypeMenu;
typedef TypeMenu<MovablePoint> PointTypeMenu;

class MovablePoint : public StdSelfRef {
    friend class StdSelfRef;
protected:
    MovablePoint(BasicTransformAnimator * const parentTransform,
                 const MovablePointType &type,
                 const qreal &radius = 7.5);
public:
    virtual QPointF getRelativePos() const = 0;
    virtual void setRelativePos(const QPointF &relPos) = 0;

    virtual void scale(const qreal &scaleXBy, const qreal &scaleYBy);
    virtual void scaleRelativeToSavedPivot(const qreal &sx, const qreal &sy);
    virtual void rotateRelativeToSavedPivot(const qreal &rot);
    virtual void saveTransformPivotAbsPos(const QPointF &absPivot);

    virtual void startTransform();
    virtual void finishTransform() {}
    virtual void cancelTransform() {}

    virtual void drawSk(SkCanvas * const canvas,
                        const SkScalar &invScale);

    virtual void remove() {}
    virtual bool isVisible() const;
    virtual bool isVisible(const CanvasMode& mode) const {
        Q_UNUSED(mode);
        return isVisible();
    }

    virtual void canvasContextMenu(PointTypeMenu * const menu) {
        Q_UNUSED(menu);
    }

    void moveToAbs(const QPointF& absPos);
    void moveByAbs(const QPointF &absTrans);
    void moveToRel(const QPointF &relPos);
    void moveByRel(const QPointF &relTranslation);
    QPointF getAbsolutePos() const;
    bool isPointAtAbsPos(const QPointF &absPoint,
                         const qreal &canvasScaleInv);
    void setAbsolutePos(const QPointF &pos);

    BasicTransformAnimator *getParentTransform();

    bool isContainedInRect(const QRectF &absRect);

    void select();
    void deselect();

    void hide();
    void show();
    bool isHidden() const;
    bool isHidden(const CanvasMode& mode) const {
        return !isVisible(mode);
    }
    void setVisible(const bool &bT);

    bool isNodePoint();
    bool isSmartNodePoint();
    bool isPivotPoint();
    bool isCtrlPoint();

    void rotateBy(const qreal &rot);
    void scale(const qreal &scaleBy);
    void applyTransform(const QMatrix &transform) {
        setRelativePos(transform.map(getRelativePos()));
    }
    void setRadius(const qreal& radius);
    qreal getRadius();


    void drawHovered(SkCanvas * const canvas,
                     const SkScalar &invScale);

    QPointF mapRelativeToAbsolute(const QPointF &relPos) const;
    QPointF mapAbsoluteToRelative(const QPointF &absPos) const;

    const QPointF &getSavedRelPos() const;

    bool isSelected() const { return mSelected; }

    bool selectionEnabled() const {
        return mSelectionEnabled;
    }

    void enableSelection() {
        setSelectionEnabled(true);
    }

    void disableSelection() {
        setSelectionEnabled(false);
    }

    void setSelectionEnabled(const bool& enabled) {
        mSelectionEnabled = enabled;
    }
protected:
    void drawOnAbsPosSk(SkCanvas * const canvas,
                        const SkPoint &absPos,
                        const SkScalar &invScale,
                        const SkColor &fillColor,
                        const bool &keyOnCurrent = false);
private:
    bool mSelectionEnabled = true;
    bool mSelected = false;
    bool mTransformStarted = false;
    bool mVisible = true;
    const MovablePointType mType;
    qreal mRadius;
    QPointF mSavedTransformPivot;
    QPointF mSavedRelPos;
    BasicTransformAnimator * const mParentTransform_cv;
};
#endif // MOVABLEPOINT_H
