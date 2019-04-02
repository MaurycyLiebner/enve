#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "Animators/qpointfanimator.h"
#include "skia/skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"

class BoundingBox;
class NodePoint;
class BasicTransformAnimator;

class PropertyUpdater;

enum MovablePointType : short {
    TYPE_PATH_POINT,
    TYPE_SMART_PATH_POINT,
    TYPE_CTRL_POINT,
    TYPE_PIVOT_POINT,
    TYPE_GRADIENT_POINT,
    TYPE_BONE_POINT
};
class PointTypeMenu;
class MovablePoint : public StdSelfRef {
    friend class StdSelfRef;
public:
    ~MovablePoint() {}

    virtual QPointF getRelativePos() const = 0;
    virtual void setRelativePos(const QPointF &relPos) = 0;
    virtual void applyTransform(const QMatrix &transform) = 0;

    virtual void moveToAbs(const QPointF& absPos);
    virtual void moveByAbs(const QPointF &absTrans);

    virtual void scale(const qreal &scaleXBy, const qreal &scaleYBy);
    virtual void scaleRelativeToSavedPivot(const qreal &sx, const qreal &sy);
    virtual void rotateRelativeToSavedPivot(const qreal &rot);
    virtual void saveTransformPivotAbsPos(const QPointF &absPivot);

    virtual void startTransform();
    virtual void finishTransform();
    virtual void cancelTransform() {}

    virtual void drawSk(SkCanvas * const canvas,
                        const SkScalar &invScale);

    virtual bool selectionEnabled() const {
        return true;
    }
    virtual void removeFromVectorPath() {}
    virtual bool isHidden() const;

    virtual void canvasContextMenu(PointTypeMenu * const menu) {
        Q_UNUSED(menu);
    }

    void moveByRel(const QPointF &relTranslation) {
        setRelativePos(mSavedRelPos + relTranslation);
    }
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
    bool isVisible() const;
    void setVisible(const bool &bT);

    bool isNodePoint();
    bool isSmartNodePoint();
    bool isPivotPoint();
    bool isCtrlPoint();
    bool isBonePoint();

    void rotateBy(const qreal &rot);
    void scale(const qreal &scaleBy);

    void setRadius(const qreal& radius);

    qreal getRadius();
    void moveToRel(const QPointF &relPos);

    void drawHovered(SkCanvas *canvas,
                     const SkScalar &invScale);

    QPointF mapRelativeToAbsolute(const QPointF &relPos) const;
    QPointF mapAbsoluteToRelative(const QPointF &absPos) const;

    const QPointF &getSavedRelPos() const;

    bool isSelected() const { return mSelected; }
protected:
    MovablePoint(BasicTransformAnimator* parentTransform,
                 const MovablePointType &type,
                 const qreal &radius = 7.5);

    bool mSelected = false;
    bool mTransformStarted = false;
    bool mHidden = false;
    MovablePointType mType;
    qreal mRadius;
    QPointF mSavedTransformPivot;
    QPointF mSavedRelPos;
    BasicTransformAnimator* mParentTransform_cv = nullptr;

    void drawOnAbsPosSk(SkCanvas * const canvas,
                        const SkPoint &absPos,
                        const SkScalar &invScale,
                        const SkColor &fillColor,
                        const bool &keyOnCurrent = false);
};
#endif // MOVABLEPOINT_H
