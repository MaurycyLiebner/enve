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

#ifndef RANDOMQREALGENERATOR_H
#define RANDOMQREALGENERATOR_H
#include "qrealvalueeffect.h"
class IntAnimator;
class ComboBoxProperty;

class RandomQrealGenerator : public QrealValueEffect {
    e_OBJECT
protected:
    RandomQrealGenerator();
public:
    qreal getDevAtRelFrame(const qreal relFrame);
    void anim_setAbsFrame(const int frame);
    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
private:
    qsptr<QrealAnimator> mTime;
    qsptr<QrealAnimator> mSmoothness;
    qsptr<QrealAnimator> mMaxDev;
    qsptr<IntAnimator> mSeedAssist;
    qsptr<ComboBoxProperty> mType;
};

#endif // RANDOMQREALGENERATOR_H
