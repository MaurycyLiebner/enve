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

#ifndef INTERPOLATIONANIMATORT_H
#define INTERPOLATIONANIMATORT_H
#include "graphanimatort.h"
#include "qrealpoint.h"
#include "interpolationkeyt.h"

template <typename T, typename K = InterpolationKeyT<T>>
class InterpolationAnimatorT :
        public BasedAnimatorT<GraphAnimator, K, T> {
    e_OBJECT
public:
    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType type,
            qreal &minValue, qreal &maxValue) const override {
        if(type == QrealPointType::keyPt) {
            minValue = key->getRelFrame();
            maxValue = minValue;
            //getFrameConstraints(key, type, minValue, maxValue);
        } else {
            minValue = -DBL_MAX;
            maxValue = DBL_MAX;
        }
    }

protected:
    InterpolationAnimatorT(const QString& name) :
        BasedAnimatorT<GraphAnimator, K, T>(name) {}

    T getValueAtRelFrameK(const qreal frame,
                          const K * const prevKey,
                          const K * const nextKey) const override {
        T result;
        const qreal prevFrame = prevKey->getRelFrame();
        const qreal nextFrame = nextKey->getRelFrame();
        const qCubicSegment1D seg{prevFrame,
                                  prevKey->getC1Frame(),
                                  nextKey->getC0Frame(),
                                  nextFrame};
        const qreal t = gTFromX(seg, frame);
        const qreal p0y = prevKey->getValueForGraph();
        const qreal p1y = prevKey->getC1Value();
        const qreal p2y = nextKey->getC0Value();
        const qreal p3y = nextKey->getValueForGraph();
        const qreal iFrame = gCubicValueAtT({p0y, p1y, p2y, p3y}, t);
        const qreal tEff = (iFrame - prevFrame)/(nextFrame - prevFrame);
        gInterpolate(prevKey->getValue(), nextKey->getValue(),
                     tEff, result);
        return result;
    }
};

#endif // INTERPOLATIONANIMATORT_H
