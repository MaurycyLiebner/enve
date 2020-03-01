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

#include "coloranimatorbinding.h"

#include <QJSEngine>

#include "Animators/coloranimator.h"

ColorAnimatorBinding::ColorAnimatorBinding(const Validator& validator,
                                           const Property* const context) :
    PropertyBinding(sWrapValidatorForClass<ColorAnimator>(validator),
                    context) {}

QJSValue toArray(QJSEngine& e, const QColor& value) {
    auto array = e.newArray(4);
    array.setProperty(0, value.redF());
    array.setProperty(1, value.greenF());
    array.setProperty(2, value.blueF());
    array.setProperty(3, value.alphaF());
    return array;
}

QJSValue ColorAnimatorBinding::getJSValue(QJSEngine& e) {
    return toArray(e, getValue());
}

QJSValue ColorAnimatorBinding::getJSValue(QJSEngine& e, const qreal relFrame) {
    return toArray(e, getValue(relFrame));
}

void ColorAnimatorBinding::updateValue() {
    const auto anim = static_cast<ColorAnimator*>(getBindProperty());
    if(!anim || !isBindPathValid()) return;
    mCurrentValue = anim->getColor();
}

QColor ColorAnimatorBinding::getValue() {
    updateValueIfNeeded();
    return mCurrentValue;
}

QColor ColorAnimatorBinding::getValue(const qreal relFrame) {
    const auto anim = static_cast<ColorAnimator*>(getBindProperty());
    return anim->getColor(relFrame);
}
