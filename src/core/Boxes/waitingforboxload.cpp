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

#include "waitingforboxload.h"
#include "Boxes/boundingbox.h"

WaitingForBoxLoad::WaitingForBoxLoad(const int boxReadId,
                                     const BoxReadFunc &boxRead,
                                     const BoxNeverReadFunc &boxNeverRead) :
    mBoxReadId(boxReadId),
    mBoxReadFunction(boxRead),
    mBoxNeverReadFunction(boxNeverRead) {}

bool WaitingForBoxLoad::boxRead(BoundingBox * const box) const {
    if(box->getReadId() != mBoxReadId) return false;
    if(mBoxReadFunction) mBoxReadFunction(box);
    return true;
}

void WaitingForBoxLoad::boxNeverRead() const {
    if(mBoxNeverReadFunction) mBoxNeverReadFunction();
}
