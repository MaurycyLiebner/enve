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

#include "drawpath.h"

#include "Segments/fitcurves.h"
#include "pointhelpers.h"
#include "Segments/smoothcurves.h"

DrawPath::DrawPath() {}

void DrawPath::lineTo(const QPointF& pos) {
    if(!mPts.isEmpty()) {
        const QPointF& lastPos = mPts.last();
        const QPointF dPos = pos - lastPos;
        const qreal dist = pointToLen(dPos);
        const int n = qFloor(dist/2);
        const qreal div = 1./(1 + n);
        for(int i = 1; i <= n; i++) {
            mPts.append(lastPos + i*dPos*div);
        }
    }
    mPts.append(pos);
}

QPointF operator*(const QPointF& p1, const QPointF& p2);

QPointF operator/(const QPointF& p1, const QPointF& p2) {
    return {p1.x()/p2.x(), p1.y()/p2.y()};
}

void DrawPath::fit(const int window, const qreal maxError) {
    mFitted.clear();
    QVector<QPointF> smooth;
    SmoothCurves::movingAverage(mPts, smooth, true, window);
    if(smooth.count() < 2) return;

    const auto adder = [this](const int n, const BezierCurve curve) {
        Q_UNUSED(n)
        const auto qptData = reinterpret_cast<QPointF*>(curve);
        const QPointF& p0 = qptData[0];
        const QPointF& c1 = qptData[1];
        const QPointF& c2 = qptData[2];
        const QPointF& p3 = qptData[3];

        mFitted.append(qCubicSegment2D{p0, c1, c2, p3});
    };
    FitCurves::FitCurve(smooth, maxError, adder);
}

void DrawPath::clear() {
    mPts.clear();
    mFitted.clear();
}
