#include "movablepoint.h"
#include "Boxes/vectorpath.h"
#include "global.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include "skqtconversions.h"
#include "pointhelpers.h"

MovablePoint::MovablePoint(BoundingBox *parent,
                           const MovablePointType &type,
                           const qreal &radius) :
    Transformable() {
    mType = type;
    mRadius = radius;
    mParent = parent;
}

void MovablePoint::startTransform() {
    mTransformStarted = true;
    mSavedRelPos = getRelativePos();
}

void MovablePoint::drawHovered(SkCanvas *canvas,
                               const qreal &invScale) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2.*invScale);
    paint.setColor(SK_ColorRED);
    canvas->drawCircle(QPointFToSkPoint(getAbsolutePos()),
                       mRadius*invScale, paint);
    //pen.setCosmetic(true);
    //p->setPen(pen);
//    drawCosmeticEllipse(p, getAbsolutePos(),
//                        mRadius, mRadius);
}

void MovablePoint::finishTransform() {
    if(mTransformStarted) {
        mTransformStarted = false;
    }
}

void MovablePoint::setAbsolutePos(const QPointF &pos) {
    QPointF newPos = mParent->getCombinedTransform().inverted().map(pos);
    setRelativePos(newPos);
}

QPointF MovablePoint::mapRelativeToAbsolute(const QPointF &relPos) const {
    return mParent->mapRelPosToAbs(relPos);
}

QPointF MovablePoint::mapAbsoluteToRelative(const QPointF &absPos) const {
    return mParent->mapAbsPosToRel(absPos);
}

QPointF MovablePoint::getAbsolutePos() const {
    return mapRelativeToAbsolute(getRelativePos());
}

void MovablePoint::drawOnAbsPosSk(SkCanvas *canvas,
                const SkPoint &absPos,
                const SkScalar &invScale,
                const unsigned char r,
                const unsigned char g,
                const unsigned char b) {
    canvas->save();

    SkScalar scaledRadius = mRadius*invScale;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SkColorSetARGBInline(255, r, g, b));

    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(invScale);
    canvas->drawCircle(absPos,
                       scaledRadius, paint);

//    if(prp_isKeyOnCurrentFrame()) {
//        paint.setColor(SK_ColorRED);
//        paint.setStyle(SkPaint::kFill_Style);
//        canvas->drawCircle(absPos,
//                           scaledRadius*0.5, paint);

//        paint.setStyle(SkPaint::kStroke_Style);
//        paint.setColor(SK_ColorBLACK);
//        canvas->drawCircle(absPos,
//                           scaledRadius*0.5, paint);
//    }
    canvas->restore();
}

void MovablePoint::drawSk(SkCanvas *canvas,
                          const SkScalar &invScale) {
    if(isHidden()) {
        return;
    }
    if(mSelected) {
        drawOnAbsPosSk(canvas,
                       QPointFToSkPoint(getAbsolutePos()),
                       invScale,
                       255, 0, 0);
    } else {
        drawOnAbsPosSk(canvas,
                       QPointFToSkPoint(getAbsolutePos()),
                       invScale,
                       255, 175, 175);
    }
}

BoundingBox *MovablePoint::getParent() {
    return mParent;
}

bool MovablePoint::isPointAtAbsPos(const QPointF &absPoint,
                                   const qreal &canvasScaleInv) {
    if(isHidden()) {
        return false;
    }
    QPointF dist = getAbsolutePos() - absPoint;
    return (dist.x()*dist.x() + dist.y()*dist.y() <
            mRadius*mRadius*canvasScaleInv*canvasScaleInv);
}

bool MovablePoint::isContainedInRect(const QRectF &absRect) {
    if(isHidden() ||
            (isCtrlPoint()) ) {
        return false;
    }
    return absRect.contains(getAbsolutePos());
}

void MovablePoint::rotateRelativeToSavedPivot(const qreal &rot) {
    QMatrix mat;
    mat.translate(mSavedTransformPivot.x(),
                  mSavedTransformPivot.y());
    mat.rotate(rot);
    mat.translate(-mSavedTransformPivot.x(),
                  -mSavedTransformPivot.y());
    moveToRel(mat.map(mSavedRelPos));
}

void MovablePoint::scaleRelativeToSavedPivot(const qreal &sx,
                                             const qreal &sy) {
    QMatrix mat;
    mat.translate(mSavedTransformPivot.x(),
                  mSavedTransformPivot.y());
    mat.scale(sx, sy);
    mat.translate(-mSavedTransformPivot.x(),
                  -mSavedTransformPivot.y());
    moveToRel(mat.map(mSavedRelPos));
}

void MovablePoint::saveTransformPivotAbsPos(const QPointF &absPivot) {
    mSavedTransformPivot = mParent->mapAbsPosToRel(absPivot);
}

void MovablePoint::rotateBy(const qreal &rot) {
    QMatrix rotMatrix;
    rotMatrix.translate(-mSavedTransformPivot.x(),
                        -mSavedTransformPivot.y());
    rotMatrix.rotate(rot);
    rotMatrix.translate(mSavedTransformPivot.x(),
                        mSavedTransformPivot.y());
    setRelativePos(rotMatrix.map(mSavedRelPos));
}

void MovablePoint::scale(const qreal &scaleXBy,
                         const qreal &scaleYBy) {
    QMatrix scaleMatrix;
    scaleMatrix.translate(-mSavedTransformPivot.x(),
                          -mSavedTransformPivot.y());
    scaleMatrix.scale(scaleXBy, scaleYBy);
    scaleMatrix.translate(mSavedTransformPivot.x(),
                          mSavedTransformPivot.y());
    setRelativePos(scaleMatrix.map(mSavedRelPos));
}

void MovablePoint::moveToRel(const QPointF &relPos) {
    moveByRel(relPos - mSavedRelPos);
}

void MovablePoint::moveByAbs(const QPointF &absTranslatione) {
    moveToAbs(mapRelativeToAbsolute(mSavedRelPos) +
              absTranslatione);
}

void MovablePoint::moveToAbs(QPointF absPos) {
    setAbsolutePos(absPos);
}

void MovablePoint::scale(const qreal &scaleBy) {
    scale(scaleBy, scaleBy);
}

void MovablePoint::cancelTransform() {
    //prp_cancelTransform();
}

void MovablePoint::setRadius(qreal radius)
{
    mRadius = radius;
}

bool MovablePoint::isBeingTransformed()
{
    return mSelected || mParent->isSelected();
}

qreal MovablePoint::getRadius()
{
    return mRadius;
}

//void MovablePoint::saveTransformPivot(QPointF absPivot)
//{
//    mSavedTransformPivot = -mParent->mapAbsPosToRel(absPivot);
//}

void MovablePoint::select() {
    mSelected = true;
}

void MovablePoint::deselect() {
    mSelected = false;
}

void MovablePoint::removeFromVectorPath() {

}

void MovablePoint::hide() {
    mHidden = true;
    deselect();
}

void MovablePoint::show() {
    mHidden = false;
}

bool MovablePoint::isHidden() {
    return mHidden;
}

bool MovablePoint::isVisible() {
    return !isHidden();
}

void MovablePoint::setVisible(const bool &bT) {
    if(bT) {
        show();
    } else {
        hide();
    }
}

bool MovablePoint::isNodePoint() {
    return mType == MovablePointType::TYPE_PATH_POINT;
}

bool MovablePoint::isPivotPoint() {
    return mType == MovablePointType::TYPE_PIVOT_POINT;
}

bool MovablePoint::isCtrlPoint() {
    return mType == MovablePointType::TYPE_CTRL_POINT;
}
