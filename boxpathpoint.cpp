#include "boxpathpoint.h"
#include "Boxes/boundingbox.h"
#include "pointhelpers.h"

BoxPathPoint::BoxPathPoint(BoundingBox *box) :
    MovablePoint(box, TYPE_PATH_POINT, 7.) {
    anim_setTraceKeyOnCurrentFrame(true);
}

void BoxPathPoint::startTransform() {
    MovablePoint::startTransform();
    mSavedAbsPos = getAbsolutePos();
    mParent->startPivotTransform();
}

void BoxPathPoint::finishTransform() {
    MovablePoint::finishTransform();
    mParent->finishPivotTransform();
}

void BoxPathPoint::moveByAbs(const QPointF &absTranslatione) {
    QPointF absPos = mSavedAbsPos + absTranslatione;
    mParent->setPivotAbsPos(absPos, false, true);
}


void BoxPathPoint::draw(QPainter *p) {
    if(mHidden) {
        return;
    }
    p->save();
    QPointF absPos = getAbsolutePos();
//    p->save();
//    p->setBrush(Qt::red);
//    p->drawPath(mMappedRotationPath);
//    p->restore();

    if(mSelected) {
        p->setBrush(QColor(255, 255, 0));
    } else {
        p->setBrush(QColor(255, 255, 125));
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

    p->translate(absPos);
    p->scale(1./p->transform().m11(), 1./p->transform().m22());
    qreal halfRadius = mRadius*0.5;
    p->drawLine(QPointF(-halfRadius, 0), QPointF(halfRadius, 0));
    p->drawLine(QPointF(0, -halfRadius), QPointF(0, halfRadius));
    p->restore();
}

void BoxPathPoint::drawSk(SkCanvas *canvas,
                          const SkScalar &invScale) {
    if(isHidden()) {
        return;
    }
    SkPoint absPos = QPointFToSkPoint(getAbsolutePos());
    if(mSelected) {
        drawOnAbsPosSk(canvas, absPos, invScale,
                       255, 255, 0);
    } else {
        drawOnAbsPosSk(canvas, absPos, invScale,
                       255, 255, 125);
    }

    canvas->save();
    canvas->translate(absPos.x(), absPos.y());
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorBLACK);
    SkScalar scaledHalfRadius = mRadius*invScale*0.5;
    canvas->drawLine(-scaledHalfRadius, 0., scaledHalfRadius, 0., paint);
    canvas->drawLine(0, -scaledHalfRadius, 0., scaledHalfRadius, paint);
    canvas->restore();
}
