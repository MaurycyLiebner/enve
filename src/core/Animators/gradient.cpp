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

#include "gradient.h"
#include "Animators/coloranimator.h"
#include "Boxes/pathbox.h"

int Gradient::sNextDocumnetId = 0;

Gradient::Gradient() : DynamicComplexAnimator<ColorAnimator>("gradient"),
    mDocumentId(sNextDocumnetId++) {
    connect(this, &Property::prp_currentFrameChanged,
            this, &Gradient::updateQGradientStops);
}

Gradient::Gradient(const QColor &color1, const QColor &color2) :
    Gradient() {
    addColor(color1);
    addColor(color2);
}

void Gradient::write(const int id, eWriteStream& dst) {
    mReadWriteId = id;
    dst << id;
    prp_writeProperty(dst);
}

int Gradient::read(eReadStream& src) {
    src >> mReadWriteId;
    prp_readProperty(src);
    return mReadWriteId;
}

void Gradient::prp_setInheritedFrameShift(
        const int shift, ComplexAnimator *parentAnimator) {
    Q_UNUSED(shift)
    if(!parentAnimator) return;
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        parentAnimator->ca_updateDescendatKeyFrame(key);
    }
}

QColor Gradient::getLastQGradientStopQColor() {
    if(mQGradientStops.isEmpty()) return Qt::black;
    return mQGradientStops.last().second;
}

QColor Gradient::getFirstQGradientStopQColor() {
    if(mQGradientStops.isEmpty()) return Qt::black;
    return mQGradientStops.first().second;
}

void Gradient::addColor(const QColor &color) {
    auto newColorAnimator = enve::make_shared<ColorAnimator>();
    newColorAnimator->setColor(color);
    addChild(newColorAnimator);
}

void Gradient::replaceColor(const int id, const QColor &color) {
    const auto colorAnim = getChild(id);
    colorAnim->setColor(color);
}

void Gradient::startColorIdTransform(const int id) {
    const auto color = getChild(id);
    color->prp_startTransform();
}

QColor Gradient::getColorAt(const int id) {
    const int nCols = ca_getNumberOfChildren();
    if(id < 0 || id >= nCols) return Qt::black;
    const auto color = getChild(id);
    return color->getColor();
}

QGradientStops Gradient::getQGradientStopsAtAbsFrame(const qreal absFrame) {
    const int nCols = ca_getNumberOfChildren();
    QGradientStops stops;
    const qreal inc = 1./(nCols - 1);
    qreal cPos = 0.;
    for(int i = 0; i < nCols; i++) {
        const auto colorAnim = getChild(i);
        const auto color = colorAnim->getColor(absFrame);
        const auto stop = QPair<qreal, QColor>(clamp(cPos, 0, 1), color);
        stops.append(stop);
        cPos += inc;
    }
    return stops;
}

void Gradient::updateQGradientStops() {
    const int nCols = ca_getNumberOfChildren();
    mQGradientStops.clear();
    const qreal inc = 1./(nCols - 1);
    qreal cPos = 0;
    for(int i = 0; i < nCols; i++) {
        const auto colorAnim = getChild(i);
        const auto color = colorAnim->getColor();
        const auto stop = QPair<qreal, QColor>(clamp(cPos, 0, 1), color);
        mQGradientStops.append(stop);
        cPos += inc;
    }

    emit changed();
}
