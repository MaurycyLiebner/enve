#include "pathpivot.h"
#include "canvas.h"
#include "pointhelpers.h"
#include "Animators/transformanimator.h"

PathPivot::PathPivot(Canvas *parent) :
    NonAnimatedMovablePoint(parent->getTransformAnimator(),
                            TYPE_PIVOT_POINT) {
    setRadius(7);
    mCanvas = parent;
//    mRotationPath.addEllipse(QPointF(0., 0.), 50., 50.);
//    QPainterPath removeEllipse;
//    removeEllipse.addEllipse(QPointF(0., 0.), 40., 40.);
//    mRotationPath -= removeEllipse;
}

void PathPivot::drawSk(SkCanvas * const canvas,
                       const CanvasMode &mode,
                       const SkScalar &invScale,
                       const bool &keyOnCurrent) {
    Q_UNUSED(keyOnCurrent);
    const SkPoint absPos = toSkPoint(getAbsolutePos());
    if(isVisible(mode)) {
        const SkColor fillCol = isSelected() ?
                    SkColorSetRGB(0, 255, 0) :
                    SkColorSetRGB(125, 255, 125);
        const SkPoint absPos = toSkPoint(getAbsolutePos());
        drawOnAbsPosSk(canvas, absPos, invScale, fillCol);
    }

    canvas->save();
    canvas->translate(absPos.x(), absPos.y());
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    SkScalar scaledHalfRadius = toSkScalar(getRadius()*0.5)*invScale;
    canvas->drawLine(-scaledHalfRadius, 0, scaledHalfRadius, 0, paint);
    canvas->drawLine(0, -scaledHalfRadius, 0, scaledHalfRadius, paint);
    canvas->restore();
}

bool PathPivot::isVisible(const CanvasMode &mode) const {
    if(mCanvas->getPivotLocal()) return false;
    if(mode == MOVE_POINT) return !mCanvas->isPointSelectionEmpty();
    else if(mode == MOVE_BOX) return !mCanvas->isBoxSelectionEmpty();
    return false;
}

bool PathPivot::isRotating() {
    return mRotating;
}

bool PathPivot::isScaling() {
    return mScaling;
}

void PathPivot::startRotating() {
    mRotating = true;
    mLastDRot = 0;
    mRotHalfCycles = 0;
}

void PathPivot::startScaling() {
    mScaling = true;
}

bool PathPivot::handleMousePress(const QPointF &absPos,
                                 const CanvasMode &mode,
                                 const qreal &invScale) {
    if(isPointAtAbsPos(absPos, mode, invScale)) {
        select();
        return true;
    }
    return false;
}

bool PathPivot::handleMouseRelease() {
    if(mRotating) {
        mRotating = false;
        return true;
    } else if(mScaling) {
        mScaling = false;
        return true;
    }
    return false;
}

qreal signedSquare(qreal val) {
    int mult;
    if(val > 0.) {
        mult = 1;
    } else {
        mult = -1;
    }
    return val*val*mult;
}

qreal distSign(QPointF distPt) {
    qreal val = signedSquare(distPt.x()) + signedSquare(distPt.y() );
    if(val > 0.) {
        return sqrt(val);
    } else {
        return -sqrt(-val);
    }
}

bool PathPivot::handleMouseMove(const QPointF &moveDestAbs,
                                const QPointF &pressPos,
                                const bool &xOnly,
                                const bool &yOnly,
                                const bool &inputTransformationEnabled,
                                const qreal &inputTransformationValue,
                                const bool &startTransform,
                                const CanvasMode &mode) {
    if(mRotating) {
        QPointF absPos = getAbsolutePos();
        qreal rot;
        if(inputTransformationEnabled) {
            rot = inputTransformationValue;
        } else {
            QLineF dest_line(absPos, moveDestAbs);
            QLineF prev_line(absPos, pressPos);
            qreal d_rot = dest_line.angleTo(prev_line);
            if(d_rot > 180)
            {
                d_rot -= 360;
            }

            if(mLastDRot - d_rot > 90) {
                mRotHalfCycles += 2;
            } else if(mLastDRot - d_rot < -90) {
                mRotHalfCycles -= 2;
            }
            mLastDRot = d_rot;
            rot = d_rot + mRotHalfCycles*180.;
        }

        if(mode == CanvasMode::MOVE_BOX) {
            mCanvas->rotateSelectedBy(rot, absPos, startTransform);
        } else {
            mCanvas->rotateSelectedPointsBy(rot, absPos, startTransform);
        }
        return true;
    } else if(mScaling) {
        QPointF absPos = getAbsolutePos();
        QPointF distMoved = moveDestAbs - pressPos;

        //if(scaleBy < 0.) scaleBy = 0.;
        qreal scaleBy;
        if(inputTransformationEnabled) {
            scaleBy = inputTransformationValue;
        } else {
            scaleBy = 1. + distSign(distMoved)*0.003;
        }
        qreal scaleX;
        qreal scaleY;
        if(xOnly) {
            scaleX = scaleBy;
            scaleY = 1.;
        } else if(yOnly) {
            scaleX = 1.;
            scaleY = scaleBy;
        } else {
            scaleX = scaleBy;
            scaleY = scaleBy;
        }

        if(mode == CanvasMode::MOVE_BOX) {
            mCanvas->scaleSelectedBy(scaleX, scaleY, absPos, startTransform);
        } else {
            mCanvas->scaleSelectedPointsBy(scaleX, scaleY, absPos, startTransform);
        }
        return true;
    }
    return false;
}

//bool PathPivot::isRotationPathAt(QPointF absPos) {
//    return mMappedRotationPath.contains(absPos);
//}
