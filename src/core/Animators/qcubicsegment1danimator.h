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

#ifndef QCUBICSEGMENT1DANIMATOR_H
#define QCUBICSEGMENT1DANIMATOR_H
#include "Animators/interpolationanimatort.h"
#include "Segments/qcubicsegment1d.h"
#include "../ReadWrite/basicreadwrite.h"
class qCubicSegment1DAnimator;

class qCubicSegment1DAnimator : public InterpolationAnimatorT<qCubicSegment1D> {
    e_OBJECT
    Q_OBJECT
public:
    qCubicSegment1DAnimator(const QString &name);

    bool SWT_isQCubicSegment1DAnimator() const { return true; }
signals:
    void currentValueChanged(qCubicSegment1D);
protected:
    void afterValueChanged() {
        emit currentValueChanged(mCurrentValue);
    }
};

class qCubicSegment1DAction {
    enum Type { START, SET, FINISH, CANCEL };
    qCubicSegment1DAction(const qCubicSegment1D value, const Type type) :
        mValue(value), mType(type) {}
public:
    void apply(qCubicSegment1DAnimator* const target) const {
        if(mType == START) target->prp_startTransform();
        else if(mType == SET) target->setCurrentValue(mValue);
        else if(mType == FINISH) target->prp_finishTransform();
        else if(mType == CANCEL) target->prp_cancelTransform();
    }

    static qCubicSegment1DAction sMakeStart()
    { return qCubicSegment1DAction{0., START}; }
    static qCubicSegment1DAction sMakeSet(const qreal value)
    { return qCubicSegment1DAction{value, SET}; }
    static qCubicSegment1DAction sMakeFinish()
    { return qCubicSegment1DAction{0., FINISH}; }
    static qCubicSegment1DAction sMakeCencel()
    { return qCubicSegment1DAction{0., CANCEL}; }
private:
    qCubicSegment1D mValue;
    Type mType;
};


#endif // QCUBICSEGMENT1DANIMATOR_H
