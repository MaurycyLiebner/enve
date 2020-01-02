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

#include "switchbutton.h"

SwitchButton::SwitchButton(const QString &toolTip, QWidget *parent) :
    ButtonBase(toolTip, parent) {

}

SwitchButton::~SwitchButton() {
    for(const auto state : mStates) {
        delete state;
    }
}

SwitchButton *SwitchButton::sCreate2Switch(
        const QString &icon0, const QString &icon1,
        const QString &toolTip, QWidget *parent) {
    const auto butt = new SwitchButton(toolTip, parent);
    butt->addState(icon0); butt->addState(icon1);
    return butt;
}

bool SwitchButton::setState(const int state) {
    if(state < 0) return false;
    if(state >= mStates.count()) return false;
    mCurrentState = state;
    setCurrentIcon(*mStates.at(mCurrentState));
    return true;
}

int SwitchButton::addState(const QString &icon) {
    mStates << new QImage(icon);
    if(mStates.count() == 1) toggle();
    return mStates.count() - 1;
}

void SwitchButton::toggle() {
    mCurrentState++;
    if(mCurrentState >= mStates.count()) mCurrentState = 0;
    setCurrentIcon(*mStates.at(mCurrentState));
    emit toggled(mCurrentState);
}
