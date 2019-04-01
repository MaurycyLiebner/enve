#include "movablepoint.h"
#include "global.h"
#include "undoredo.h"
#include "Boxes/boxesgroup.h"
#include "skia/skqtconversions.h"
#include "pointhelpers.h"
#include "Animators/transformanimator.h"

MovablePoint::MovablePoint(BasicTransformAnimator* parentTransform,
                           const MovablePointType &type,
                           const qreal &radius) {
    mType = type;
    mRadius = radius;
    setParentTransformAnimator(parentTransform);
}

void MovablePoint::startTransform() {
    mTransformStarted = true;
    mSavedRelPos = getRelativePos();
}

const QPointF &MovablePoint::getSavedRelPos() const {
    return mSavedRelPos;
}

void MovablePoint::drawHovered(SkCanvas *canvas,
                               const SkScalar &invScale) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2.f*invScale);
    paint.setColor(SK_ColorRED);
    canvas->drawCircle(toSkPoint(getAbsolutePos()),
                       static_cast<SkScalar>(mRadius)*invScale, paint);
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
    setRelativePos(mParentTransform_cv->mapAbsPosToRel(pos));
}

QPointF MovablePoint::mapRelativeToAbsolute(const QPointF &relPos) const {
    return mParentTransform_cv->mapRelPosToAbs(relPos);
}

QPointF MovablePoint::mapAbsoluteToRelative(const QPointF &absPos) const {
    return mParentTransform_cv->mapAbsPosToRel(absPos);
}

QPointF MovablePoint::getAbsolutePos() const {
    return mapRelativeToAbsolute(getRelativePos());
}

void MovablePoint::drawOnAbsPosSk(
        SkCanvas * const canvas,
        const SkPoint &absPos,
        const SkScalar &invScale,
        const SkColor& fillColor,
        const bool &keyOnCurrent) {
    canvas->save();

    const SkScalar scaledRadius = static_cast<SkScalar>(mRadius)*invScale;

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(fillColor);

    paint.setStyle(SkPaint::kFill_Style);
    canvas->drawCircle(absPos, scaledRadius, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(invScale);
    canvas->drawCircle(absPos, scaledRadius, paint);

    if(keyOnCurrent) {
        const SkScalar halfRadius = scaledRadius*0.5f;

        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawCircle(absPos, halfRadius, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0.5f);
        paint.setColor(SK_ColorBLACK);
        canvas->drawCircle(absPos, halfRadius, paint);
    }
    canvas->restore();
}

void MovablePoint::drawSk(SkCanvas * const canvas,
                          const SkScalar &invScale) {
    if(isHidden()) return;
    const SkColor fillCol = mSelected ?
                SkColorSetRGB(255, 0, 0) :
                SkColorSetRGB(255, 175, 175);
    const SkPoint absPos = toSkPoint(getAbsolutePos());
    drawOnAbsPosSk(canvas, absPos, invScale, fillCol);
}

BasicTransformAnimator *MovablePoint::getParentTransform() {
    return mParentTransform_cv;
}

bool MovablePoint::isPointAtAbsPos(const QPointF &absPoint,
                                   const qreal &canvasScaleInv) {
    if(isHidden()) return false;
    const QPointF dist = getAbsolutePos() - absPoint;
    return pointToLen(dist) < mRadius*canvasScaleInv;
}

bool MovablePoint::isContainedInRect(const QRectF &absRect) {
    if(isHidden() || isCtrlPoint()) return false;
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
    mSavedTransformPivot = mParentTransform_cv->mapAbsPosToRel(absPivot);
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

void MovablePoint::moveToAbs(const QPointF& absPos) {
    setAbsolutePos(absPos);
}

void MovablePoint::scale(const qreal &scaleBy) {
    scale(scaleBy, scaleBy);
}

void MovablePoint::setRadius(const qreal& radius) {
    mRadius = radius;
}

qreal MovablePoint::getRadius() {
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

bool MovablePoint::isHidden() const {
    return mHidden;
}

bool MovablePoint::isVisible() const {
    return !isHidden();
}

void MovablePoint::setVisible(const bool &bT) {
    if(bT) show();
    else hide();
}

bool MovablePoint::isNodePoint() {
    return mType == MovablePointType::TYPE_PATH_POINT;
}

bool MovablePoint::isSmartNodePoint() {
    return mType == MovablePointType::TYPE_SMART_PATH_POINT;
}

bool MovablePoint::isPivotPoint() {
    return mType == MovablePointType::TYPE_PIVOT_POINT;
}

bool MovablePoint::isCtrlPoint() {
    return mType == MovablePointType::TYPE_CTRL_POINT;
}

bool MovablePoint::isBonePoint() {
    return mType == MovablePointType::TYPE_BONE_POINT;
}
