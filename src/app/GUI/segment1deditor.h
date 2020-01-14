// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SEGMENT1DEDITOR_H
#define SEGMENT1DEDITOR_H

#include <QWidget>
#include "Segments/qcubicsegment1d.h"
#include "Animators/qcubicsegment1danimator.h"
#include "conncontextptr.h"
class Segment1DEditor : public QWidget {
    Q_OBJECT
    enum PressedPt {
        P0, C1, C2, P1, NONE
    };

public:
    explicit Segment1DEditor(const qreal minY, const qreal maxY,
                             QWidget *parent = nullptr);
    void setValueRange(const qreal minY, const qreal maxY);
    void setCurrentAnimator(qCubicSegment1DAnimator * const animator);
    void setCurrentSegment(const qCubicSegment1D& seg);
    void setMargin(const qreal margin);
    void setPointRadius(const qreal radius);
    void sendValueToAnimator();
protected:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent*);
    void wheelEvent(QWheelEvent* e);
private:
    void updateAfterAnimatorChanged();
    void updateDrawPath();
    QPointF valueToPos(const QPointF &value) const;
    qreal xValueToXPos(const qreal x) const;
    qreal yValueToYPos(const qreal y) const;
    QPointF valueToPos(const qreal valX, const qreal valY) const;
    QPointF posToValue(const QPointF &pos) const;
    QPointF p0() const;
    QPointF c1() const;
    QPointF c2() const;
    QPointF p1() const;
    ConnContextQPtr<qCubicSegment1DAnimator> mCurrentAnimator;
    qCubicSegment1D mCurrentSegment;
    QPainterPath mCurrentDrawPath;
    qreal mMinY;
    qreal mMaxY;
    PressedPt mPressedPt;
    QPointF mPressedPos;
    QPointF mPressedPtValue;
    QPointF mPressedValue;
    qreal mPtRad = 3;
    qreal mMargin = 3;
    qreal mAddMarginY;
    qreal mTopMargin = 0;
    void clampTopMarginAddMarginY();
signals:
    void editingCanceled();
    void editingStarted();
    void editingFinished();
    void segmentEdited(qCubicSegment1D);
    void segmentChanged(qCubicSegment1D);
};

#endif // SEGMENT1DEDITOR_H
