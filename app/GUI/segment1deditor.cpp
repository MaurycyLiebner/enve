#include "segment1deditor.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include "pointhelpers.h"

Segment1DEditor::Segment1DEditor(const qreal &minY, const qreal &maxY,
                                 QWidget *parent) : QWidget(parent),
    mCurrentSegment(minY, 1./3*(maxY - minY) + minY,
                    2./3*(maxY - minY) + minY, maxY) {
    mMinY = minY;
    mMaxY = maxY;
    mAddMarginY = mMargin + mPtRad;
    setMinimumHeight(100);
    //updateDrawPath();
}

qreal Segment1DEditor::xValueToXPos(const qreal& x) const {
    qreal wid = width() - 2*mPtRad - 2*mMargin;
    return x*wid + mPtRad + mMargin;
}

qreal Segment1DEditor::yValueToYPos(const qreal &y) const {
    qreal hig = height() - 2*mPtRad - 2*mMargin - 2*mAddMarginY;
    return height() - ((y - mMinY)*hig/(mMaxY - mMinY) +
                    mPtRad + mMargin + mAddMarginY + mTopMargin);
}

QPointF Segment1DEditor::valueToPos(const QPointF& value) const {
    qreal wid = width() - 2*mPtRad - 2*mMargin;
    qreal hig = height() - 2*mPtRad - 2*mMargin - 2*mAddMarginY;
    QPointF pos = QPointF(value.x()*wid,
                          (value.y() - mMinY)*hig/(mMaxY - mMinY)) +
            QPointF(mPtRad + mMargin,
                    mPtRad + mMargin + mAddMarginY + mTopMargin);
    return QPointF(pos.x(), height() - pos.y());
}

QPointF Segment1DEditor::valueToPos(const qreal &valX,
                                    const qreal &valY) const {
    return valueToPos(QPointF(valX, valY));
}

QPointF Segment1DEditor::posToValue(const QPointF& pos) const {
    qreal wid = width() - 2*mPtRad - 2*mMargin;
    qreal hig = height() - 2*mPtRad - 2*mMargin - 2*mAddMarginY;
    QPointF posNoMar = QPointF(pos.x(), height() - pos.y()) -
            QPointF(mPtRad + mMargin,
                    mPtRad + mMargin + mAddMarginY + mTopMargin);
    return QPointF(posNoMar.x()/wid,
                   posNoMar.y()*(mMaxY - mMinY)/hig + mMinY);
}

QPointF Segment1DEditor::p0() const {
    return QPointF(0, mCurrentSegment.p0());
}

QPointF Segment1DEditor::c1() const {
    return QPointF(1./3, mCurrentSegment.c1());
}

QPointF Segment1DEditor::c2() const {
    return QPointF(2./3, mCurrentSegment.c2());
}

QPointF Segment1DEditor::p1() const {
    return QPointF(1, mCurrentSegment.p1());
}

void Segment1DEditor::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::white);
    p.setBrush(Qt::NoBrush);
    QPen pen(p.pen());
    pen.setStyle(Qt::DotLine);
    pen.setColor(Qt::lightGray);
    p.setPen(pen);

    for(int i = 0; i <= 10; i++) {
        const QPointF p1 = valueToPos(QPointF(0, mMinY + i*(mMaxY - mMinY)/10));
        p.drawLine(QPointF(mMargin, p1.y()),
                   QPointF(width() - mMargin, p1.y()));
    }
    pen.setStyle(Qt::SolidLine);
    pen.setColor(Qt::black);
    p.setPen(pen);
    p.drawPath(mCurrentDrawPath);
    const QPointF p0Pos(valueToPos(0, mCurrentSegment.p0()));
    const QPointF c1Pos(valueToPos(1./3, mCurrentSegment.c1()));
    const QPointF c2Pos(valueToPos(2./3, mCurrentSegment.c2()));
    const QPointF p1Pos(valueToPos(1, mCurrentSegment.p1()));

    pen.setStyle(Qt::DotLine);
    pen.setColor(Qt::black);
    p.setPen(pen);
    p.drawLine(p0Pos, c1Pos);
    p.drawLine(p1Pos, c2Pos);

    p.setPen(Qt::NoPen);
    if(mCurrentAnimator) {
        if(mCurrentAnimator->anim_getKeyOnCurrentFrame()) {
            p.setBrush(Qt::red);
        } else {
            p.setBrush(Qt::black);
        }
    } else {
        p.setBrush(Qt::black);
    }
    p.drawEllipse(p0Pos, mPtRad, mPtRad);
    p.drawEllipse(c1Pos, mPtRad, mPtRad);
    p.drawEllipse(c2Pos, mPtRad, mPtRad);
    p.drawEllipse(p1Pos, mPtRad, mPtRad);

    p.end();
}

void Segment1DEditor::mousePressEvent(QMouseEvent *e) {
    const QPointF pos = e->pos();
    mPressedPos = pos;
    mPressedValue = posToValue(pos);
    const QPointF p0Pos = valueToPos(p0());
    const QPointF c1Pos = valueToPos(c1());
    const QPointF c2Pos = valueToPos(c2());
    const QPointF p1Pos = valueToPos(p1());

    const qreal p0Dist = pointToLen(p0Pos - pos);
    const qreal c1Dist = pointToLen(c1Pos - pos);
    const qreal c2Dist = pointToLen(c2Pos - pos);
    const qreal p1Dist = pointToLen(p1Pos - pos);

    PressedPt closestPt = P0;
    qreal minDist = p0Dist;
    mPressedPtValue = p0();
    if(c1Dist < minDist) {
        minDist = c1Dist;
        closestPt = C1;
        mPressedPtValue = c1();
    }
    if(c2Dist < minDist) {
        minDist = c2Dist;
        closestPt = C2;
        mPressedPtValue = c2();
    }
    if(p1Dist < minDist) {
        minDist = p1Dist;
        closestPt = P1;
        mPressedPtValue = p1();
    }
    if(minDist < 2*mPtRad) {
        mPressedPt = closestPt;
    } else {
        mPressedPt = NONE;
    }
//    if(c1Dist < mPtRad) {
//        mPressedPt = C1;
//        mPressedPtValue = c1();
//    } else if(c2Dist < mPtRad) {
//        mPressedPt = C2;
//        mPressedPtValue = c2();
//    } else if(p1Dist < mPtRad) {
//        mPressedPt = P1;
//        mPressedPtValue = p1();
//    } else if(p0Dist < mPtRad) {
//        mPressedPt = P0;
//        mPressedPtValue = p0();
//    } else {
//        PressedPt closestPt = P0;
//        qreal minDist = p0Dist;
//        if(c1Dist < minDist) {
//            minDist = c1Dist;
//            closestPt = C1;
//        }
//        if(c2Dist < minDist) {
//            minDist = c2Dist;
//            closestPt = C2;
//        }
//        if(p1Dist < minDist) {
//            minDist = p1Dist;
//            closestPt = P1;
//        }
//        if(minDist < 2*mPtRad) {
//            mPressedPt = closestPt;
//        } else {
//            mPressedPt = NONE;
//        }
//    }
}

void Segment1DEditor::mouseMoveEvent(QMouseEvent *e) {
    if(mPressedPt == NONE) return;
    const qreal hig = height() - 2*mPtRad - 2*mMargin - 2*mAddMarginY;
    qreal newVal = mPressedPtValue.y() +
            (mPressedPos.y() - e->y())*(mMaxY - mMinY)/hig;
    newVal = CLAMP(newVal, mMinY, mMaxY);
    if(mPressedPt == P0) {
        mCurrentSegment.setP0(newVal);
    } else if(mPressedPt == C1) {
        mCurrentSegment.setC1(newVal);
    } else if(mPressedPt == C2) {
        mCurrentSegment.setC2(newVal);
    } else if(mPressedPt == P1) {
        mCurrentSegment.setP1(newVal);
    }
    sendValueToAnimator();
    updateDrawPath();
    emit segmentEdited(mCurrentSegment);
    emit segmentChanged(mCurrentSegment);
}

void Segment1DEditor::resizeEvent(QResizeEvent *) {
    clampTopMarginAddMarginY();
}

void Segment1DEditor::wheelEvent(QWheelEvent *e) {
    if(QApplication::keyboardModifiers() & Qt::ControlModifier) {
        if(e->delta() < 0) {
            mAddMarginY += 5;
        } else {
            mAddMarginY -= 5;
        }
    } else {
        if(e->delta() > 0) {
            mTopMargin += 3;
        } else {
            mTopMargin -= 3;
        }
    }
    clampTopMarginAddMarginY();
    mPressedPos = valueToPos(mPressedValue);
}

void Segment1DEditor::clampTopMarginAddMarginY() {
    mAddMarginY = CLAMP(mAddMarginY, -height()/2, mMargin + mPtRad);
    qreal totHeight = height() - 2*mAddMarginY + 2*mMargin + 2*mPtRad;
    qreal maxTrans = (totHeight - height())/2;
    if(totHeight < height()) maxTrans = 0;
    mTopMargin = CLAMP(mTopMargin, -maxTrans, maxTrans);

    updateDrawPath();
}

void Segment1DEditor::updateDrawPath() {
    mCurrentDrawPath = QPainterPath();
    mCurrentDrawPath.moveTo(valueToPos(0, mCurrentSegment.p0()));
    mCurrentDrawPath.cubicTo(valueToPos(1./3, mCurrentSegment.c1()),
                             valueToPos(2./3, mCurrentSegment.c2()),
                             valueToPos(1, mCurrentSegment.p1()));
    update();
}
