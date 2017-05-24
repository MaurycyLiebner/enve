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
    QPointFAnimator(),
    Transformable() {
    mType = type;
    mRadius = radius;
    mParent = parent;
}

void MovablePoint::startTransform()
{
    prp_startTransform();
    mTransformStarted = true;
    mSavedRelPos = getRelativePos();
}

void MovablePoint::applyTransform(const QMatrix &transform){
    setCurrentPointValue(transform.map(getCurrentPointValue()), true);
}

void MovablePoint::removeAnimations() {
    if(prp_isRecording()) {
        prp_setRecording(false);
    }
}

void MovablePoint::makeDuplicate(MovablePoint *targetPoint) {
    targetPoint->duplicatePosAnimatorFrom(this);
}

void MovablePoint::duplicatePosAnimatorFrom(QPointFAnimator *source) {
    source->makeDuplicate(this);
}

void MovablePoint::drawHovered(QPainter *p) {
    p->setBrush(Qt::NoBrush);
    QPen pen = QPen(Qt::red, 2.);
    pen.setCosmetic(true);
    p->setPen(pen);
    drawCosmeticEllipse(p, getAbsolutePos(),
                        mRadius, mRadius);
}

void MovablePoint::drawHovered(SkCanvas *canvas,
                               const SkScalar &invScale) {
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

void MovablePoint::finishTransform()
{
    if(mTransformStarted) {
        mTransformStarted = false;
        prp_finishTransform();
    }
}

void MovablePoint::setAbsolutePos(const QPointF &pos,
                                  const bool &saveUndoRedo) {
    QMatrix combinedM = mParent->getCombinedTransform();
    QPointF newPos = combinedM.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo);
}

void MovablePoint::setRelativePos(const QPointF &relPos,
                                  const bool &saveUndoRedo) {
    setCurrentPointValue(relPos, saveUndoRedo);
}

QPointF MovablePoint::getRelativePos() const
{
    return getCurrentPointValue();
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

void MovablePoint::drawOnAbsPos(QPainter *p,
                                const QPointF &absPos) {
    if(mSelected) {
        p->setBrush(QColor(255, 0, 0));
    } else {
        p->setBrush(QColor(255, 175, 175));
    }

    drawCosmeticEllipse(p, absPos,
                        mRadius, mRadius);
    if(prp_isKeyOnCurrentFrame()) {
        p->save();

        p->setBrush(Qt::red);
        QPen pen = QPen(Qt::black, 1.);
        pen.setCosmetic(true);
        p->setPen(pen);
        drawCosmeticEllipse(p, absPos,
                            mRadius*0.5, mRadius*0.5);
        p->restore();
    }
}

void MovablePoint::drawOnAbsPos(SkCanvas *canvas,
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

    if(prp_isKeyOnCurrentFrame()) {
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawCircle(absPos,
                           scaledRadius*0.5, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorBLACK);
        canvas->drawCircle(absPos,
                           scaledRadius*0.5, paint);
    }
    canvas->restore();
}

void MovablePoint::draw(QPainter *p) {
    if(isHidden()) {
        return;
    }
    drawOnAbsPos(p, getAbsolutePos());
}

void MovablePoint::draw(SkCanvas *canvas,
                        const SkScalar &invScale) {
    if(isHidden()) {
        return;
    }
    if(mSelected) {
        drawOnAbsPos(canvas,
                     QPointFToSkPoint(getAbsolutePos()),
                     invScale,
                     255, 0, 0);
    } else {
        drawOnAbsPos(canvas,
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
    if(isHidden() || (isCtrlPoint() && !BoxesGroup::getCtrlsAlwaysVisible()) ) {
        return false;
    }
    return absRect.contains(getAbsolutePos());
}

void MovablePoint::rotateRelativeToSavedPivot(const qreal &rot) {
    QMatrix mat;
    mat.translate(mSavedTransformPivot.x(), mSavedTransformPivot.y());
    mat.rotate(rot);
    mat.translate(-mSavedTransformPivot.x(), -mSavedTransformPivot.y());
    moveToRel(mat.map(mSavedRelPos));
}

void MovablePoint::scaleRelativeToSavedPivot(const qreal &sx,
                                             const qreal &sy) {
    QMatrix mat;
    mat.translate(mSavedTransformPivot.x(), mSavedTransformPivot.y());
    mat.scale(sx, sy);
    mat.translate(-mSavedTransformPivot.x(), -mSavedTransformPivot.y());
    moveToRel(mat.map(mSavedRelPos));
}

void MovablePoint::saveTransformPivotAbsPos(const QPointF &absPivot) {
    mSavedTransformPivot = mParent->mapAbsPosToRel(absPivot);
}

void MovablePoint::moveToRel(const QPointF &relPos) {
    moveByRel(relPos - mSavedRelPos);
}

void MovablePoint::moveByRel(const QPointF &relTranslation) {
    incSavedValueToCurrentValue(relTranslation.x(),
                                        relTranslation.y());
}

void MovablePoint::moveByAbs(const QPointF &absTranslatione) {
    moveToAbs(mapRelativeToAbsolute(getSavedPointValue()) +
              absTranslatione);
}

void MovablePoint::moveToAbs(QPointF absPos) {
    setAbsolutePos(absPos, false);
}

void MovablePoint::scale(const qreal &scaleBy) {
    scale(scaleBy, scaleBy);
}

void MovablePoint::cancelTransform()
{
    prp_cancelTransform();
    //setRelativePos(mSavedRelPos, false);
}

void MovablePoint::setRadius(qreal radius)
{
    mRadius = radius;
}

bool MovablePoint::isBeingTransformed()
{
    return mSelected || mParent->isSelected();
}

void MovablePoint::setPosAnimatorUpdater(AnimatorUpdater *updater) {
    prp_setUpdater(updater);
}

qreal MovablePoint::getRadius()
{
    return mRadius;
}

void MovablePoint::updateAfterFrameChanged(const int &frame) {
    prp_setAbsFrame(frame);
}

void MovablePoint::rotateBy(const qreal &rot) {
    QMatrix rotMatrix;
    rotMatrix.translate(-mSavedTransformPivot.x(), -mSavedTransformPivot.y());
    rotMatrix.rotate(rot);
    rotMatrix.translate(mSavedTransformPivot.x(), mSavedTransformPivot.y());
    setRelativePos(mSavedRelPos*rotMatrix, false);
}

void MovablePoint::scale(const qreal &scaleXBy,
                         const qreal &scaleYBy) {
    QMatrix scaleMatrix;
    scaleMatrix.translate(-mSavedTransformPivot.x(), -mSavedTransformPivot.y());
    scaleMatrix.scale(scaleXBy, scaleYBy);
    scaleMatrix.translate(mSavedTransformPivot.x(), mSavedTransformPivot.y());
    setRelativePos(mSavedRelPos*scaleMatrix, false);
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

void MovablePoint::setVisible(bool bT) {
    if(bT) {
        show();
    } else {
        hide();
    }
}

bool MovablePoint::isPathPoint() {
    return mType == MovablePointType::TYPE_PATH_POINT;
}

bool MovablePoint::isPivotPoint() {
    return mType == MovablePointType::TYPE_PIVOT_POINT;
}

bool MovablePoint::isCtrlPoint() {
    return mType == MovablePointType::TYPE_CTRL_POINT;
}
