// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#include "stacklayouts.h"

template<class SplitItemClass>
SplitItemClass* SplittableStackItem::split(WidgetPtr &&other) {
    Q_ASSERT(mParent);
    auto vStack = std::make_unique<SplitItemClass>();
    const auto vStackPtr = vStack.get();
    UniPtr thisUni = mParent->replaceChild(this, std::move(vStack));
    vStackPtr->setChildren(std::move(thisUni), std::move(other));
    return vStackPtr;
}

VSplitStackItem* SplittableStackItem::splitV(WidgetPtr &&other) {
    return split<VSplitStackItem>(std::move(other));
}

HSplitStackItem* SplittableStackItem::splitH(WidgetPtr &&other) {
    return split<HSplitStackItem>(std::move(other));
}

void ParentStackLayoutItem::sWriteChild(StackLayoutItem * const child,
                                        eWriteStream& dst) {
    child->writeType(dst);
    child->write(dst);
}
