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

#include "eeffect.h"

eEffect::eEffect(const QString &name) :
    StaticComplexAnimator(name) {
    ca_setDisabledWhenEmpty(false);
}

void eEffect::prp_writeProperty(eWriteStream& dst) const {
    StaticComplexAnimator::prp_writeProperty(dst);
    dst << mVisible;
}

void eEffect::prp_readProperty(eReadStream& src) {
    StaticComplexAnimator::prp_readProperty(src);
    bool visible; src >> visible;
    setVisible(visible);
}

void eEffect::switchVisible() {
    setVisible(!mVisible);
}

void eEffect::setVisible(const bool visible) {
    if(visible == mVisible) return;
    mVisible = visible;
    prp_afterWholeInfluenceRangeChanged();
    emit effectVisibilityChanged(visible);
}

bool eEffect::isVisible() const {
    return mVisible;
}
