#include "boxpathpoint.h"
#include "Boxes/boundingbox.h"

BoxPathPoint::BoxPathPoint(BoundingBox *box) :
    MovablePoint(box, TYPE_PATH_POINT, 7.) {

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

void BoxPathPoint::moveByAbs(QPointF absTranslatione) {
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

    p->translate(absPos);
    p->scale(1./p->transform().m11(), 1./p->transform().m22());
    qreal halfRadius = mRadius*0.5;
    p->drawLine(QPointF(-halfRadius, 0), QPointF(halfRadius, 0));
    p->drawLine(QPointF(0, -halfRadius), QPointF(0, halfRadius));
    p->restore();
}
